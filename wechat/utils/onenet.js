const STORAGE_KEYS = {
  config: 'onenet_config',
  devices: 'onenet_devices',
  debug: 'onenet_last_debug',
  product: 'onenet_product',
}

const DEFAULT_CONFIG = {
  baseUrl: 'https://iot-api.heclouds.com',
  productId: 'gqp5I5JYU8',
  authorization: '',
}

const ENDPOINTS = {
  productDetail: '/product/detail',
  deviceList: '/device/list',
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

function getProductInfo() {
  return wx.getStorageSync(STORAGE_KEYS.product) || null
}

function saveProductInfo(info) {
  wx.setStorageSync(STORAGE_KEYS.product, info)
}

function getLastDebugInfo() {
  return wx.getStorageSync(STORAGE_KEYS.debug) || null
}

function saveLastDebugInfo(info) {
  wx.setStorageSync(STORAGE_KEYS.debug, info)
}

function requestApi({ method = 'GET', path, data = {} }) {
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

        resolve({
          raw: body,
          data: body.data,
          debugInfo,
        })
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

// 1) 产品详情接口：用于初始化与校验 product_id + authorization
function getProductDetail(productId) {
  return requestApi({
    method: 'GET',
    path: ENDPOINTS.productDetail,
    data: { product_id: productId },
  })
}

// 2) 获取设备列表接口：初始化成功后获取设备 id/name
function getDeviceList(productId) {
  return requestApi({
    method: 'GET',
    path: ENDPOINTS.deviceList,
    data: { product_id: productId },
  })
}

module.exports = {
  STORAGE_KEYS,
  DEFAULT_CONFIG,
  ENDPOINTS,
  getConfig,
  saveConfig,
  getLocalDevices,
  saveLocalDevices,
  getProductInfo,
  saveProductInfo,
  getLastDebugInfo,
  getProductDetail,
  getDeviceList,
}
