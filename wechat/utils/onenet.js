const STORAGE_KEYS = {
  config: 'onenet_config',
  devices: 'onenet_devices',
  debug: 'onenet_last_debug',
}

const AUTH_MODES = {
  AUTHORIZATION_ONLY: 'authorization',
  AUTHORIZATION_BEARER: 'authorization_bearer',
  API_KEY_ONLY: 'api_key',
  BOTH: 'both',
}

const DEFAULT_CONFIG = {
  baseUrl: 'https://iot-api.heclouds.com',
  productId: 'gqp5I5JYU8',
  apiKey: '',
  authMode: AUTH_MODES.AUTHORIZATION_ONLY,
  debug: true,
}

// 统一维护接口路径，后续若文档更新仅需改这里
const ENDPOINTS = {
  listDevices: '/thingmodel/query-devices',
  queryDeviceDetail: '/thingmodel/query-device-detail',
  createDevice: '/thingmodel/create-device',
  deleteDevice: '/thingmodel/delete-device',
}

function getConfig() {
  const saved = wx.getStorageSync(STORAGE_KEYS.config) || {}
  return {
    ...DEFAULT_CONFIG,
    ...saved,
  }
}

function saveConfig(config) {
  wx.setStorageSync(STORAGE_KEYS.config, {
    ...getConfig(),
    ...config,
  })
}

function getLocalDevices() {
  return wx.getStorageSync(STORAGE_KEYS.devices) || []
}

function saveLocalDevices(devices) {
  wx.setStorageSync(STORAGE_KEYS.devices, devices)
}

function saveLastDebugInfo(debugInfo) {
  wx.setStorageSync(STORAGE_KEYS.debug, debugInfo)
}

function getLastDebugInfo() {
  return wx.getStorageSync(STORAGE_KEYS.debug) || null
}

function buildHeaders(config) {
  const token = (config.apiKey || '').trim()
  const headers = {
    'content-type': 'application/json',
  }

  if (!token) {
    return headers
  }

  if (config.authMode === AUTH_MODES.AUTHORIZATION_ONLY) {
    headers.authorization = token
  } else if (config.authMode === AUTH_MODES.AUTHORIZATION_BEARER) {
    headers.authorization = `Bearer ${token}`
  } else if (config.authMode === AUTH_MODES.API_KEY_ONLY) {
    headers['api-key'] = token
  } else if (config.authMode === AUTH_MODES.BOTH) {
    headers.authorization = token
    headers['api-key'] = token
  }

  return headers
}

function extractMessage(body, statusCode) {
  return body.msg || body.message || body.error || body.err_msg || `HTTP ${statusCode}`
}

function isBodySuccess(body) {
  if (body === null || body === undefined) return true
  if (typeof body !== 'object') return true

  // 常见成功语义
  if (body.code === 0 || body.errno === 0 || body.success === true) return true

  // 常见失败语义
  if (typeof body.code === 'number' && body.code !== 0) return false
  if (typeof body.errno === 'number' && body.errno !== 0) return false
  if (body.success === false) return false

  // 兼容无 code 字段但 HTTP 为 2xx 的情况
  return true
}

function normalizeData(body) {
  if (body === null || body === undefined) return {}
  if (typeof body !== 'object') return { raw: body }
  if (body.data !== undefined) return body.data
  if (body.result !== undefined) return body.result
  return body
}

function requestApi({ method = 'GET', path, data = {}, requireToken = true, customHeaders = {} }) {
  const config = getConfig()

  if (requireToken && !config.apiKey) {
    return Promise.reject(new Error('请先在设备管理页配置 OneNET Token/API Key'))
  }

  const url = `${config.baseUrl}${path}`
  const headers = {
    ...buildHeaders(config),
    ...customHeaders,
  }

  const reqMeta = {
    url,
    method,
    requestData: data,
    requestHeaders: headers,
    time: new Date().toISOString(),
    authMode: config.authMode,
  }

  return new Promise((resolve, reject) => {
    wx.request({
      url,
      method,
      data,
      header: headers,
      success: (res) => {
        const body = res.data || {}
        const okHttp = res.statusCode >= 200 && res.statusCode < 300
        const okBody = isBodySuccess(body)

        const debugInfo = {
          ...reqMeta,
          statusCode: res.statusCode,
          responseHeaders: res.header || {},
          responseBody: body,
          responseDataExtracted: normalizeData(body),
          success: okHttp && okBody,
        }

        if (config.debug) {
          saveLastDebugInfo(debugInfo)
        }

        if (!okHttp || !okBody) {
          const msg = extractMessage(body, res.statusCode)
          const err = new Error(msg)
          err.debugInfo = debugInfo
          reject(err)
          return
        }

        resolve({
          raw: body,
          data: normalizeData(body),
          debugInfo,
        })
      },
      fail: (err) => {
        const debugInfo = {
          ...reqMeta,
          success: false,
          networkError: err,
        }

        if (config.debug) {
          saveLastDebugInfo(debugInfo)
        }

        const wrappedErr = new Error(err.errMsg || '网络请求失败')
        wrappedErr.debugInfo = debugInfo
        reject(wrappedErr)
      },
    })
  })
}

function listDevices() {
  const { productId } = getConfig()
  return requestApi({
    method: 'GET',
    path: ENDPOINTS.listDevices,
    data: { product_id: productId },
  })
}

function queryDeviceDetail(deviceName) {
  const { productId } = getConfig()
  return requestApi({
    method: 'GET',
    path: ENDPOINTS.queryDeviceDetail,
    data: { product_id: productId, device_name: deviceName },
  })
}

function createDevice(deviceName) {
  const { productId } = getConfig()
  return requestApi({
    method: 'POST',
    path: ENDPOINTS.createDevice,
    data: { product_id: productId, device_name: deviceName },
  })
}

function deleteDevice(deviceName) {
  const { productId } = getConfig()
  return requestApi({
    method: 'POST',
    path: ENDPOINTS.deleteDevice,
    data: { product_id: productId, device_name: deviceName },
  })
}

// 用于调试 token 是否有效；优先走列表接口
function probeToken() {
  return listDevices()
}

module.exports = {
  STORAGE_KEYS,
  AUTH_MODES,
  DEFAULT_CONFIG,
  ENDPOINTS,
  getConfig,
  saveConfig,
  getLocalDevices,
  saveLocalDevices,
  getLastDebugInfo,
  requestApi,
  listDevices,
  queryDeviceDetail,
  createDevice,
  deleteDevice,
  probeToken,
}
