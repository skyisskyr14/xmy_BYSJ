const oneNet = require('../../utils/onenet')

const keyAlias = {
  turbidity: 'turd',
  turd: 'turd',
  tds: 'tds',
  temp: 'temp',
  ph: 'ph',
}

Page({
  data: {
    key: '',
    identifier: '',
    title: '',
    unit: '',
    current: '',
    deviceName: '01',
    range: 'minute',
    rangeTabs: [
      { key: 'minute', text: '分钟' },
      { key: 'hour', text: '小时' },
      { key: 'day', text: '天' },
    ],
    chartBars: [],
    statMin: '-',
    statMax: '-',
    statAvg: '-',
    historyRows: [],
    loading: false,
  },

  onLoad(options) {
    const { key = 'turd', title = '指标', unit = '', value = '', deviceName = '01' } = options
    const identifier = keyAlias[key] || key

    this.setData({ key, identifier, title, unit, current: value, deviceName })
    wx.setNavigationBarTitle({ title: `${title}历史` })
    this.fetchHistory('minute')
  },

  switchRange(e) {
    this.fetchHistory(e.currentTarget.dataset.range)
  },

  getTimeRange(range) {
    const end = Date.now()
    if (range === 'minute') return { start: end - 60 * 60 * 1000, end }
    if (range === 'hour') return { start: end - 24 * 60 * 60 * 1000, end }
    return { start: end - 7 * 24 * 60 * 60 * 1000, end }
  },

  async fetchHistory(range) {
    this.setData({ loading: true, range })
    const cfg = oneNet.getConfig()
    const { start, end } = this.getTimeRange(range)

    try {
      const res = await oneNet.getPropertyHistory({
        productId: cfg.productId,
        deviceName: this.data.deviceName,
        identifier: this.data.identifier,
        startTime: start,
        endTime: end,
        limit: '30',
        sort: '2',
      })

      const list = res.data?.list || []
      const values = list
        .map((x) => Number(x.value))
        .filter((x) => !Number.isNaN(x))
        .reverse()

      if (!values.length) {
        this.setData({ chartBars: [], historyRows: [], statMin: '-', statMax: '-', statAvg: '-' })
        return
      }

      const max = Math.max(...values)
      const min = Math.min(...values)
      const avg = values.reduce((s, v) => s + v, 0) / values.length

      const chartBars = values.map((val, idx) => ({
        label: `${idx + 1}`,
        value: val,
        height: `${Math.max(12, Math.round((val / (max || 1)) * 120))}rpx`,
      }))

      const historyRows = list.map((x) => ({
        time: x.time || x.ts || '-',
        value: x.value,
      }))

      this.setData({
        chartBars,
        historyRows,
        statMin: min.toFixed(2),
        statMax: max.toFixed(2),
        statAvg: avg.toFixed(2),
      })
    } catch (err) {
      wx.showToast({ title: err.message || '历史查询失败', icon: 'none' })
    } finally {
      this.setData({ loading: false })
    }
  },

  goWarning() {
    const devices = oneNet.getLocalDevices()
    const matched = devices.find((d) => d.name === this.data.deviceName)
    const deviceId = matched ? matched.id : this.data.deviceName
    wx.navigateTo({ url: `/pages/warning/warning?deviceId=${deviceId}` })
  },
})
