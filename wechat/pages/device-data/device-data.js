const oneNet = require('../../utils/onenet')

function asMap(list = []) {
  const m = {}
  list.forEach((x) => {
    m[x.identifier] = x
  })
  return m
}

Page({
  data: {
    deviceName: '',
    productId: '',
    autoEnabled: true,
    intervalSec: 5,
    loading: false,
    lastUpdate: '-',
    properties: [],
    timerId: null,
  },

  onLoad(options) {
    const cfg = oneNet.getConfig()
    const rf = oneNet.getAutoRefreshConfig()
    this.setData({
      deviceName: options.deviceName || '01',
      productId: cfg.productId,
      autoEnabled: !!rf.enabled,
      intervalSec: Number(rf.intervalSec) || 5,
    })
    wx.setNavigationBarTitle({ title: `${this.data.deviceName} 总数据` })
  },

  onShow() {
    this.refreshNow()
    this.startAutoIfNeeded()
  },

  onHide() {
    this.stopAuto()
  },

  onUnload() {
    this.stopAuto()
  },

  startAutoIfNeeded() {
    this.stopAuto()
    if (!this.data.autoEnabled) return

    const tid = setInterval(() => {
      this.refreshNow()
    }, Math.max(1, this.data.intervalSec) * 1000)
    this.setData({ timerId: tid })
  },

  stopAuto() {
    if (this.data.timerId) {
      clearInterval(this.data.timerId)
      this.setData({ timerId: null })
    }
  },

  onToggleAuto(e) {
    const enabled = e.detail.value
    this.setData({ autoEnabled: enabled })
    oneNet.saveAutoRefreshConfig({ enabled })
    this.startAutoIfNeeded()
  },

  async refreshNow() {
    this.setData({ loading: true })
    try {
      const res = await oneNet.getDeviceLatestProperties(this.data.productId, this.data.deviceName)
      const list = Array.isArray(res.data) ? res.data : []
      this.setData({
        properties: list,
        lastUpdate: new Date().toLocaleString(),
      })
    } catch (err) {
      wx.showToast({ title: err.message || '刷新失败', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  openPropertyHistory(e) {
    const { identifier, name, value } = e.currentTarget.dataset
    wx.navigateTo({
      url: `/pages/metric/metric?key=${identifier}&title=${name || identifier}&value=${value || ''}&deviceName=${this.data.deviceName}`,
    })
  },

  goRefreshConfig() {
    wx.navigateTo({ url: '/pages/refresh-config/refresh-config' })
  },
})
