Page({
  data: {
    globalEnabled: true,
    noticeMethods: [
      { key: 'toast', name: '本地提醒', enabled: true },
      { key: 'subscribe', name: '订阅消息', enabled: false },
      { key: 'sms', name: '短信提醒', enabled: false },
    ],
    rules: [
      { key: 'turbidity', name: '浊度 (NTU)', enabled: true, min: '0', max: '30' },
      { key: 'tds', name: 'TDS (ppm)', enabled: true, min: '100', max: '450' },
      { key: 'temp', name: '水温 (℃)', enabled: true, min: '18', max: '30' },
      { key: 'ph', name: 'pH', enabled: true, min: '6.5', max: '8.5' },
    ],
  },

  onToggleGlobal(e) {
    this.setData({ globalEnabled: e.detail.value })
  },

  onToggleMethod(e) {
    const index = Number(e.currentTarget.dataset.index)
    const list = [...this.data.noticeMethods]
    list[index].enabled = e.detail.value
    this.setData({ noticeMethods: list })
  },

  onToggleRule(e) {
    const index = Number(e.currentTarget.dataset.index)
    const rules = [...this.data.rules]
    rules[index].enabled = e.detail.value
    this.setData({ rules })
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
    wx.showToast({ title: '已保存（演示）', icon: 'success' })
  },
})
