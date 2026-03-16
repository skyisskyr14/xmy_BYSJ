const STORAGE_KEYS = {
  config: 'onenet_config',
  devices: 'onenet_devices',
}

const DEFAULT_CONFIG = {
  baseUrl: 'https://iot-api.heclouds.com',
  productId: 'gqp5I5JYU8',
  apiKey: '',
}

function getConfig() {
  const saved = wx.getStorageSync(STORAGE_KEYS.config) || {}
  return {
    ...DEFAULT_CONFIG,
    ...saved,
  }
}

function saveConfig(config) {
  wx.setStorageSync(STORAGE_KEYS.config, config)
}

function getLocalDevices() {
  return wx.getStorageSync(STORAGE_KEYS.devices) || []
}

function saveLocalDevices(devices) {
  wx.setStorageSync(STORAGE_KEYS.devices, devices)
}

function requestApi({ method = 'GET', path, data = {} }) {
  const config = getConfig()

  if (!config.apiKey) {
    return Promise.reject(new Error('请先在设备管理页配置 OneNET API Key'))
  }

  return new Promise((resolve, reject) => {
    wx.request({
      url: `${config.baseUrl}${path}`,
      method,
      data,
      header: {
        'content-type': 'application/json',
        authorization: config.apiKey,
      },
      success: (res) => {
        const body = res.data || {}
        if (res.statusCode >= 200 && res.statusCode < 300) {
          resolve(body)
          return
        }
        reject(new Error(body.msg || body.message || `HTTP ${res.statusCode}`))
      },
      fail: (err) => reject(err),
    })
  })
}

// 注：以下路径根据 OneNET REST 命名习惯预置，若你环境路径不同，可在此统一替换。
function listDevices() {
  const { productId } = getConfig()
  return requestApi({
    method: 'GET',
    path: '/thingmodel/query-devices',
    data: { product_id: productId },
  })
}

function queryDeviceDetail(deviceName) {
  const { productId } = getConfig()
  return requestApi({
    method: 'GET',
    path: '/thingmodel/query-device-detail',
    data: { product_id: productId, device_name: deviceName },
  })
}

function createDevice(deviceName) {
  const { productId } = getConfig()
  return requestApi({
    method: 'POST',
    path: '/thingmodel/create-device',
    data: { product_id: productId, device_name: deviceName },
  })
}

function deleteDevice(deviceName) {
  const { productId } = getConfig()
  return requestApi({
    method: 'POST',
    path: '/thingmodel/delete-device',
    data: { product_id: productId, device_name: deviceName },
  })
}

module.exports = {
  STORAGE_KEYS,
  DEFAULT_CONFIG,
  getConfig,
  saveConfig,
  getLocalDevices,
  saveLocalDevices,
  listDevices,
  queryDeviceDetail,
  createDevice,
  deleteDevice,
}
