const oneNet = require('../../utils/onenet')

const WARNING_STORAGE_KEY = 'warning_configs'

const defaultRuleTemplate = [
  { key: 'turbidity', name: '浊度 (NTU)', enabled: true, min: '0', max: '30' },
  { key: 'tds', name: 'TDS (ppm)', enabled: true, min: '100', max: '450' },
  { key: 'temp', name: '水温 (℃)', enabled: true, min: '18', max: '30' },
  { key: 'ph', name: 'pH', enabled: true, min: '6.5', max: '8.5' },
]

const defaultNoticeTemplate = [
  { key: 'toast', name: '本地提醒', enabled: true },
  { key: 'subscribe', name: '订阅消息', enabled: false },
  { key: 'sms', name: '短信提醒', enabled: false },
]

function deepCloneConfig(config) {
  return {
    enabled: config.enabled,
    noticeMethods: config.noticeMethods.map((item) => ({ ...item })),
    rules: config.rules.map((item) => ({ ...item })),
  }
}

function createDefaultConfig() {
  return {
    enabled: true,
    noticeMethods: defaultNoticeTemplate.map((item) => ({ ...item })),
    rules: defaultRuleTemplate.map((item) => ({ ...item })),
  }
}

Page({
  data: {
    devices: [],
    currentDeviceId: '',
    currentDeviceName: '',
    globalEnabled: true,
    noticeMethods: [],
    rules: [],
  },

  onLoad(options) {
    const devices = this.loadDevices()
    const fromDeviceId = options.deviceId || (devices[0] && devices[0].id)
    if (!fromDeviceId) {
      wx.showToast({ title: '暂无设备，请先在设备页添加', icon: 'none' })
      return
    }
    this.switchDevice(fromDeviceId)
  },

  loadDevices() {
    const local = oneNet.getLocalDevices()
    const devices = local.length
      ? local.map((d) => ({ id: d.id, name: d.name || d.id }))
      : [{ id: 'ESP32S3-WQ-001', name: '1号水箱' }]
    this.setData({ devices })
    return devices
  },

  getAllConfigMap() {
    return wx.getStorageSync(WARNING_STORAGE_KEY) || {}
  },

  saveAllConfigMap(map) {
    wx.setStorageSync(WARNING_STORAGE_KEY, map)
  },

  getDeviceConfig(deviceId) {
    const map = this.getAllConfigMap()
    return map[deviceId] ? deepCloneConfig(map[deviceId]) : createDefaultConfig()
  },

  saveCurrentDeviceConfig() {
    const map = this.getAllConfigMap()
    map[this.data.currentDeviceId] = {
      enabled: this.data.globalEnabled,
      noticeMethods: this.data.noticeMethods,
      rules: this.data.rules,
    }
    this.saveAllConfigMap(map)
  },

  switchDeviceByTap(e) {
    const { id } = e.currentTarget.dataset
    this.switchDevice(id)
  },

  switchDevice(deviceId) {
    if (this.data.currentDeviceId) {
      this.saveCurrentDeviceConfig()
    }

    const matchedDevice = this.data.devices.find((item) => item.id === deviceId) || this.data.devices[0]
    if (!matchedDevice) {
      return
    }

    const config = this.getDeviceConfig(matchedDevice.id)

    this.setData({
      currentDeviceId: matchedDevice.id,
      currentDeviceName: matchedDevice.name,
      globalEnabled: config.enabled,
      noticeMethods: config.noticeMethods,
      rules: config.rules,
    })

    wx.setNavigationBarTitle({ title: `${matchedDevice.name}预警` })
  },

  onToggleGlobal(e) {
    this.setData({ globalEnabled: e.detail.value })
    this.saveCurrentDeviceConfig()
  },

  onToggleMethod(e) {
    const index = Number(e.currentTarget.dataset.index)
    const list = [...this.data.noticeMethods]
    list[index].enabled = e.detail.value
    this.setData({ noticeMethods: list })
    this.saveCurrentDeviceConfig()
  },

  onToggleRule(e) {
    const index = Number(e.currentTarget.dataset.index)
    const rules = [...this.data.rules]
    rules[index].enabled = e.detail.value
    this.setData({ rules })
    this.saveCurrentDeviceConfig()
  },

  onInputMin(e) {
    const index = Number(e.currentTarget.dataset.index)
    const rules = [...this.data.rules]
    rules[index].min = e.detail.value
    this.setData({ rules })
  },

  onInputMax(e) {
    const index = Number(e.currentTarget.dataset.index)
    const rules = [...this.data.rules]
    rules[index].max = e.detail.value
    this.setData({ rules })
  },

  saveRules() {
    this.saveCurrentDeviceConfig()
    wx.showToast({ title: `${this.data.currentDeviceName}已保存`, icon: 'success' })
  },
})
