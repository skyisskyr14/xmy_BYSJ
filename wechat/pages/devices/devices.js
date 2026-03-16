const oneNet = require('../../utils/onenet')

function stringifyDebug(data) {
  if (!data) return '暂无调试信息'
  try {
    return JSON.stringify(data, null, 2)
  } catch (e) {
    return String(data)
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
    debugLogsText: '',
  },

  onLoad() {
    this.loadConfig()
    this.loadLocalData()
  },

  onShow() {
    this.loadLocalData()
    this.refreshDebugLogs()
  },

  loadConfig() {
    const config = oneNet.getConfig()
    this.setData({
      productId: config.productId,
      authorization: config.authorization,
      debugText: stringifyDebug(oneNet.getLastDebugInfo()),
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

  refreshDebugLogs() {
    const logs = oneNet.getDebugLogs()
    this.setData({ debugLogsText: stringifyDebug(logs) })
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

  async retryAutoBootstrap() {
    if (!this.data.productId || !this.data.authorization) {
      wx.showToast({ title: '请填写产品ID和authorization', icon: 'none' })
      return
    }

    this.saveConfig()
    this.setData({ loading: true })

    try {
      const res = await oneNet.bootstrapAuto()
      this.setData({
        productInfo: res.product,
        devices: res.devices,
        initialized: true,
        debugText: stringifyDebug(oneNet.getLastDebugInfo()),
      })
      this.refreshDebugLogs()
      wx.showToast({ title: '自动验证并同步成功', icon: 'success' })
    } catch (err) {
      this.setData({ debugText: stringifyDebug(err.debugInfo || err) })
      this.refreshDebugLogs()
      wx.showToast({ title: err.message || '自动同步失败', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  clearDebug() {
    oneNet.clearDebugLogs()
    this.refreshDebugLogs()
    wx.showToast({ title: '已清空调试记录', icon: 'success' })
  },

  goDeviceWarning(e) {
    const { id } = e.currentTarget.dataset
    wx.navigateTo({ url: `/pages/warning/warning?deviceId=${id}` })
  },

  goDeviceData(e) {
    const { name } = e.currentTarget.dataset
    wx.navigateTo({ url: `/pages/device-data/device-data?deviceName=${name}` })
  },
})
