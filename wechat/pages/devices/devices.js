const oneNet = require('../../utils/onenet')

function normalizeDevice(item) {
  return {
    id: String(item.did || item.name || ''),
    name: item.name || String(item.did || ''),
    status: item.status === 0 ? '在线' : item.status === 2 ? '离线' : '未知',
    latest: item.last_time || '-',
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
    productId: 'gqp5I5JYU8',
    authorization: '',

    loading: false,
    initialized: false,

    productInfo: null,
    devices: [],
    debugText: '',
  },

  onLoad() {
    this.loadConfig()
    this.loadLocalData()
  },

  onShow() {
    this.loadLocalData()
  },

  loadConfig() {
    const config = oneNet.getConfig()
    this.setData({
      productId: config.productId,
      authorization: config.authorization,
      debugText: stringifyDebug(),
    })
  },

  loadLocalData() {
    const productInfo = oneNet.getProductInfo()
    const devices = oneNet.getLocalDevices()
    this.setData({
      productInfo,
      devices,
      initialized: !!productInfo,
    })
  },

  onInputProductId(e) {
    this.setData({ productId: e.detail.value.trim() })
  },

  onInputAuthorization(e) {
    this.setData({ authorization: e.detail.value.trim() })
  },

  saveConfig() {
    oneNet.saveConfig({
      productId: this.data.productId,
      authorization: this.data.authorization,
    })
    wx.showToast({ title: '配置已保存', icon: 'success' })
  },

  showDebug(errOrRes) {
    this.setData({ debugText: stringifyDebug(errOrRes) })
  },

  async initProduct() {
    if (!this.data.productId || !this.data.authorization) {
      wx.showToast({ title: '请填写产品ID和authorization', icon: 'none' })
      return
    }

    this.saveConfig()
    this.setData({ loading: true })

    try {
      const res = await oneNet.getProductDetail(this.data.productId)
      this.showDebug(res)
      oneNet.saveProductInfo(res.data)
      this.setData({
        productInfo: res.data,
        initialized: true,
      })
      wx.showToast({ title: '产品初始化成功', icon: 'success' })
    } catch (err) {
      this.showDebug(err)
      wx.showToast({ title: err.message || '产品初始化失败', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  async fetchDeviceList() {
    if (!this.data.initialized) {
      wx.showToast({ title: '请先完成产品初始化', icon: 'none' })
      return
    }

    this.setData({ loading: true })
    try {
      const res = await oneNet.getDeviceList(this.data.productId)
      this.showDebug(res)
      const list = res.data?.list || []
      const devices = Array.isArray(list) ? list.map(normalizeDevice) : []
      oneNet.saveLocalDevices(devices)
      this.setData({ devices })
      wx.showToast({ title: '设备列表获取成功', icon: 'success' })
    } catch (err) {
      this.showDebug(err)
      wx.showToast({ title: err.message || '获取设备列表失败', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  goDeviceWarning(e) {
    const { id } = e.currentTarget.dataset
    wx.navigateTo({ url: `/pages/warning/warning?deviceId=${id}` })
  },
})
