const oneNet = require('../../utils/onenet')

Page({
  data: {
    enabled: true,
    intervalSec: 5,
  },

  onLoad() {
    const cfg = oneNet.getAutoRefreshConfig()
    this.setData({
      enabled: !!cfg.enabled,
      intervalSec: Number(cfg.intervalSec) || 5,
    })
  },

  onToggle(e) {
    this.setData({ enabled: e.detail.value })
  },

  onInputInterval(e) {
    this.setData({ intervalSec: Number(e.detail.value) || 5 })
  },

  saveConfig() {
    oneNet.saveAutoRefreshConfig({
      enabled: this.data.enabled,
      intervalSec: Math.max(1, this.data.intervalSec),
    })
    wx.showToast({ title: '刷新配置已保存', icon: 'success' })
  },
})
