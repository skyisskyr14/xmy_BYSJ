const oneNet = require('../../utils/onenet')

const fallbackDevices = [
  { id: 'ESP32S3-WQ-001', name: '1号水箱', group: '实验室', status: '在线', latest: '1分钟前', gateway: 'OneNET-Channel-A' },
  { id: 'ESP32S3-WQ-002', name: '2号水箱', group: '实验室', status: '在线', latest: '3分钟前', gateway: 'OneNET-Channel-A' },
]

const authModeOptions = [
  { label: 'authorization: token', value: oneNet.AUTH_MODES.AUTHORIZATION_ONLY },
  { label: 'authorization: Bearer token', value: oneNet.AUTH_MODES.AUTHORIZATION_BEARER },
  { label: 'api-key: token', value: oneNet.AUTH_MODES.API_KEY_ONLY },
  { label: 'authorization + api-key', value: oneNet.AUTH_MODES.BOTH },
]

function normalizeDevice(item) {
  const id = item.id || item.device_name || item.name
  return {
    id,
    name: item.name || item.device_name || id,
    group: item.group || '未分组',
    status: item.status || '未知',
    latest: item.latest || '-',
    gateway: item.gateway || 'OneNET',
  }
}

function stringifyDebug(errOrRes) {
  const info = errOrRes?.debugInfo || errOrRes || oneNet.getLastDebugInfo()
  if (!info) return '暂无调试信息'
  try {
    return JSON.stringify(info, null, 2)
  } catch (e) {
    return String(info)
  }
}

Page({
  data: {
    groupFilter: ['全部', '实验室', '户外', '未分组'],
    currentGroup: '全部',
    devices: [],
    shownDevices: [],

    productId: 'gqp5I5JYU8',
    apiKey: '',
    baseUrl: 'https://iot-api.heclouds.com',
    authModeOptions,
    authModeIndex: 0,
    authMode: oneNet.AUTH_MODES.AUTHORIZATION_ONLY,
    enableDebug: true,
    probeDeviceName: '01',

    addDeviceName: '',
    createDeviceName: '',
    deleteDeviceName: '',

    loading: false,
    debugText: '',
  },

  onLoad() {
    this.loadConfig()
    this.loadLocalDevices()
  },

  loadConfig() {
    const config = oneNet.getConfig()
    const authModeIndex = Math.max(0, authModeOptions.findIndex((item) => item.value === config.authMode))

    this.setData({
      productId: config.productId,
      apiKey: config.apiKey,
      baseUrl: config.baseUrl,
      authMode: config.authMode,
      authModeIndex,
      enableDebug: !!config.debug,
      debugText: stringifyDebug(),
    })
  },

  loadLocalDevices() {
    let devices = oneNet.getLocalDevices()
    if (!devices.length) {
      devices = fallbackDevices
      oneNet.saveLocalDevices(devices)
    }
    this.setData({ devices })
    this.applyFilter(this.data.currentGroup)
  },

  changeGroup(e) {
    this.applyFilter(e.currentTarget.dataset.group)
  },

  applyFilter(group) {
    const shownDevices = this.data.devices.filter((item) => group === '全部' || item.group === group)
    this.setData({
      currentGroup: group,
      shownDevices,
    })
  },

  onInputProductId(e) {
    this.setData({ productId: e.detail.value })
  },

  onInputApiKey(e) {
    this.setData({ apiKey: e.detail.value.trim() })
  },

  onInputBaseUrl(e) {
    this.setData({ baseUrl: e.detail.value.trim() })
  },

  onAuthModeChange(e) {
    const index = Number(e.detail.value)
    this.setData({
      authModeIndex: index,
      authMode: authModeOptions[index].value,
    })
  },

  onDebugSwitch(e) {
    this.setData({ enableDebug: e.detail.value })
  },

  savePlatformConfig() {
    oneNet.saveConfig({
      productId: this.data.productId.trim(),
      apiKey: this.data.apiKey,
      baseUrl: this.data.baseUrl.trim(),
      authMode: this.data.authMode,
      debug: this.data.enableDebug,
    })
    wx.showToast({ title: '平台配置已保存', icon: 'success' })
  },

  showDebug(errOrRes) {
    this.setData({ debugText: stringifyDebug(errOrRes) })
  },

  onInputAddDeviceName(e) {
    this.setData({ addDeviceName: e.detail.value.trim() })
  },

  onInputCreateDeviceName(e) {
    this.setData({ createDeviceName: e.detail.value.trim() })
  },

  onInputDeleteDeviceName(e) {
    this.setData({ deleteDeviceName: e.detail.value.trim() })
  },

  onInputProbeDeviceName(e) {
    this.setData({ probeDeviceName: e.detail.value.trim() })
  },

  async probeToken() {
    this.setData({ loading: true })
    try {
      const probeName = this.data.probeDeviceName || '01'
      const res = await oneNet.probeToken(probeName)
      this.showDebug(res)
      wx.showToast({ title: 'Token可用（属性接口成功）', icon: 'success' })
    } catch (err) {
      this.showDebug(err)
      wx.showToast({ title: err.message || 'Token不可用', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  async initFromCloudList() {
    this.setData({ loading: true })
    try {
      const res = await oneNet.listDevices()
      this.showDebug(res)
      const list = res.data?.list || res.data?.devices || res.data || []
      if (!Array.isArray(list) || !list.length) {
        wx.showToast({ title: '接口无设备列表，保留本地数据', icon: 'none' })
        return
      }

      const devices = list.map(normalizeDevice)
      oneNet.saveLocalDevices(devices)
      this.setData({ devices })
      this.applyFilter(this.data.currentGroup)
      wx.showToast({ title: '设备列表已初始化', icon: 'success' })
    } catch (err) {
      this.showDebug(err)
      wx.showToast({ title: err.message || '初始化失败', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  async addExistingDevice() {
    const name = this.data.addDeviceName
    if (!name) {
      wx.showToast({ title: '请输入设备名', icon: 'none' })
      return
    }

    this.setData({ loading: true })
    try {
      const res = await oneNet.queryDeviceDetail(name)
      this.showDebug(res)
      const exists = this.data.devices.some((item) => item.id === name || item.name === name)
      if (exists) {
        wx.showToast({ title: '本地已存在该设备', icon: 'none' })
        return
      }
      const newList = [...this.data.devices, normalizeDevice({ id: name, name, status: '在线' })]
      oneNet.saveLocalDevices(newList)
      this.setData({ devices: newList, addDeviceName: '' })
      this.applyFilter(this.data.currentGroup)
      wx.showToast({ title: '添加成功', icon: 'success' })
    } catch (err) {
      this.showDebug(err)
      wx.showToast({ title: err.message || '设备不存在或查询失败', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  async createNewDevice() {
    const name = this.data.createDeviceName
    if (!name) {
      wx.showToast({ title: '请输入设备名', icon: 'none' })
      return
    }

    this.setData({ loading: true })
    try {
      const res = await oneNet.createDevice(name)
      this.showDebug(res)
      const exists = this.data.devices.some((item) => item.id === name || item.name === name)
      const newList = exists
        ? this.data.devices
        : [...this.data.devices, normalizeDevice({ id: name, name, status: '在线' })]
      oneNet.saveLocalDevices(newList)
      this.setData({ devices: newList, createDeviceName: '' })
      this.applyFilter(this.data.currentGroup)
      wx.showToast({ title: '创建设备成功', icon: 'success' })
    } catch (err) {
      this.showDebug(err)
      wx.showToast({ title: err.message || '创建设备失败', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  async deleteDevice() {
    const name = this.data.deleteDeviceName
    if (!name) {
      wx.showToast({ title: '请输入设备名', icon: 'none' })
      return
    }

    this.setData({ loading: true })
    try {
      const res = await oneNet.deleteDevice(name)
      this.showDebug(res)
      const newList = this.data.devices.filter((item) => item.id !== name && item.name !== name)
      oneNet.saveLocalDevices(newList)
      this.setData({ devices: newList, deleteDeviceName: '' })
      this.applyFilter(this.data.currentGroup)
      wx.showToast({ title: '删除成功', icon: 'success' })
    } catch (err) {
      this.showDebug(err)
      wx.showToast({ title: err.message || '删除失败', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  goDeviceWarning(e) {
    const { id } = e.currentTarget.dataset
    wx.navigateTo({ url: `/pages/warning/warning?deviceId=${id}` })
  },
})
