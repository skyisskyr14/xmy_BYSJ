const seriesMap = {
  turbidity: {
    minute: [18, 17, 15, 14, 13, 12],
    hour: [28, 24, 20, 16, 14, 12],
    day: [36, 30, 25, 20, 16, 12],
  },
  tds: {
    minute: [236, 239, 238, 237, 238, 238],
    hour: [248, 244, 241, 240, 238, 238],
    day: [260, 255, 251, 246, 241, 238],
  },
  temp: {
    minute: [24.0, 24.1, 24.2, 24.4, 24.5, 24.6],
    hour: [23.5, 23.8, 24.0, 24.2, 24.4, 24.6],
    day: [22.8, 23.1, 23.5, 23.9, 24.3, 24.6],
  },
  ph: {
    minute: [7.1, 7.1, 7.2, 7.2, 7.2, 7.2],
    hour: [7.0, 7.1, 7.1, 7.2, 7.2, 7.2],
    day: [6.9, 7.0, 7.0, 7.1, 7.1, 7.2],
  },
}

Page({
  data: {
    key: '',
    title: '',
    unit: '',
    current: '',
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
    deviceId: 'ESP32S3-WQ-001',
  },

  onLoad(options) {
    const { key = 'turbidity', title = '指标', unit = '', value = '', deviceId = 'ESP32S3-WQ-001' } = options
    this.setData({
      key,
      title,
      unit,
      current: value,
      deviceId,
    })
    wx.setNavigationBarTitle({ title: `${title}趋势` })
    this.updateChart('minute')
  },

  switchRange(e) {
    const range = e.currentTarget.dataset.range
    this.updateChart(range)
  },

  updateChart(range) {
    const key = this.data.key
    const source = seriesMap[key] || seriesMap.turbidity
    const values = source[range] || source.minute

    const max = Math.max(...values)
    const min = Math.min(...values)
    const avg = values.reduce((sum, val) => sum + val, 0) / values.length

    const chartBars = values.map((val, idx) => ({
      label: `${idx + 1}`,
      value: val,
      height: `${Math.max(12, Math.round((val / max) * 120))}rpx`,
    }))

    this.setData({
      range,
      chartBars,
      statMin: min.toFixed(2),
      statMax: max.toFixed(2),
      statAvg: avg.toFixed(2),
    })
  },

  goWarning() {
    wx.navigateTo({ url: `/pages/warning/warning?deviceId=${this.data.deviceId}` })
  },
})
