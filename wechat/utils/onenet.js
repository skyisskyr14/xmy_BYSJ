const STORAGE_KEYS = {
  config: 'onenet_config',
  devices: 'onenet_devices',
  product: 'onenet_product',
  debug: 'onenet_last_debug',
  debugLogs: 'onenet_debug_logs',
  autoRefresh: 'onenet_auto_refresh_config',
}

const DEFAULT_CONFIG = {
  baseUrl: 'https://iot-api.heclouds.com',
  productId: 'gqp5I5JYU8',
  authorization: '',
}

const DEFAULT_AUTO_REFRESH = {
  enabled: true,
  intervalSec: 5,
}

const ENDPOINTS = {
  productDetail: '/product/detail',
  deviceList: '/device/list',
  deviceProperty: '/thingmodel/query-device-property',
  propertyHistory: '/thingmodel/query-device-property-history',
}

function getConfig() {
  return {
    ...DEFAULT_CONFIG,
    ...(wx.getStorageSync(STORAGE_KEYS.config) || {}),
  }
}

function saveConfig(config) {
  wx.setStorageSync(STORAGE_KEYS.config, {
    ...getConfig(),
    ...config,
  })
}

function getAutoRefreshConfig() {
  return {
    ...DEFAULT_AUTO_REFRESH,
    ...(wx.getStorageSync(STORAGE_KEYS.autoRefresh) || {}),
  }
}

function saveAutoRefreshConfig(config) {
  wx.setStorageSync(STORAGE_KEYS.autoRefresh, {
    ...getAutoRefreshConfig(),
    ...config,
  })
}

function getLocalDevices() {
  return wx.getStorageSync(STORAGE_KEYS.devices) || []
}

function saveLocalDevices(devices) {
  wx.setStorageSync(STORAGE_KEYS.devices, devices)
}

function getProductInfo() {
  return wx.getStorageSync(STORAGE_KEYS.product) || null
}

function saveProductInfo(info) {
  wx.setStorageSync(STORAGE_KEYS.product, info)
}

function getLastDebugInfo() {
  return wx.getStorageSync(STORAGE_KEYS.debug) || null
}

function getDebugLogs() {
  return wx.getStorageSync(STORAGE_KEYS.debugLogs) || []
}

function clearDebugLogs() {
  wx.setStorageSync(STORAGE_KEYS.debugLogs, [])
}

function appendDebugLog(info) {
  const logs = getDebugLogs()
  logs.unshift(info)
  wx.setStorageSync(STORAGE_KEYS.debugLogs, logs.slice(0, 50))
}

function saveLastDebugInfo(info) {
  wx.setStorageSync(STORAGE_KEYS.debug, info)
  appendDebugLog(info)
}

function requestApi({ method = 'GET', path, data = {}, debugTag = '' }) {
  const config = getConfig()
  if (!config.authorization) {
    return Promise.reject(new Error('请先填写 authorization'))
  }

  const url = `${config.baseUrl}${path}`
  const headers = {
    authorization: config.authorization,
    Accept: '*/*',
  }

  const reqMeta = {
    debugTag,
    url,
    method,
    requestData: data,
    requestHeaders: headers,
    time: new Date().toISOString(),
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
        const okBody = body && body.code === 0

        const debugInfo = {
          ...reqMeta,
          statusCode: res.statusCode,
          responseHeaders: res.header || {},
          responseBody: body,
          success: okHttp && okBody,
        }
        saveLastDebugInfo(debugInfo)

        if (!okHttp || !okBody) {
          const msg = body.msg || body.message || `HTTP ${res.statusCode}`
          const err = new Error(msg)
          err.debugInfo = debugInfo
          reject(err)
          return
        }

        resolve({ raw: body, data: body.data, debugInfo })
      },
      fail: (err) => {
        const debugInfo = {
          ...reqMeta,
          success: false,
          networkError: err,
        }
        saveLastDebugInfo(debugInfo)
        const wrappedErr = new Error(err.errMsg || '网络请求失败')
        wrappedErr.debugInfo = debugInfo
        reject(wrappedErr)
      },
    })
  })
}

function getProductDetail(productId) {
  return requestApi({
    method: 'GET',
    path: ENDPOINTS.productDetail,
    data: { product_id: productId },
    debugTag: 'product-detail',
  })
}

function getDeviceList(productId) {
  return requestApi({
    method: 'GET',
    path: ENDPOINTS.deviceList,
    data: { product_id: productId },
    debugTag: 'device-list',
  })
}

function getDeviceLatestProperties(productId, deviceName) {
  return requestApi({
    method: 'GET',
    path: ENDPOINTS.deviceProperty,
    data: { product_id: productId, device_name: deviceName },
    debugTag: 'device-property',
  })
}

function getPropertyHistory({ productId, deviceName, identifier, startTime, endTime, sort = '2', offset = '0', limit = '30' }) {
  return requestApi({
    method: 'GET',
    path: ENDPOINTS.propertyHistory,
    data: {
      product_id: productId,
      device_name: deviceName,
      identifier,
      start_time: String(startTime),
      end_time: String(endTime),
      sort,
      offset,
      limit,
    },
    debugTag: 'property-history',
  })
}

async function bootstrapAuto() {
  const config = getConfig()
  if (!config.productId || !config.authorization) {
    throw new Error('缺少产品ID或authorization，请前往设备中心配置')
  }

  const productRes = await getProductDetail(config.productId)
  saveProductInfo(productRes.data)

  const deviceRes = await getDeviceList(config.productId)
  const list = deviceRes.data?.list || []
  const devices = Array.isArray(list)
    ? list.map((item) => ({
        id: String(item.did || item.name || ''),
        name: item.name || String(item.did || ''),
        status: item.status === 0 ? '在线' : item.status === 2 ? '离线' : '未知',
        latest: item.last_time || '-',
      }))
    : []

  saveLocalDevices(devices)
  return { product: productRes.data, devices }
}

module.exports = {
  STORAGE_KEYS,
  DEFAULT_CONFIG,
  ENDPOINTS,
  getConfig,
  saveConfig,
  getAutoRefreshConfig,
  saveAutoRefreshConfig,
  getLocalDevices,
  saveLocalDevices,
  getProductInfo,
  saveProductInfo,
  getLastDebugInfo,
  getDebugLogs,
  clearDebugLogs,
  getProductDetail,
  getDeviceList,
  getDeviceLatestProperties,
  getPropertyHistory,
  bootstrapAuto,
}
