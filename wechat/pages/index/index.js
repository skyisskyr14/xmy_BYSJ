const oneNet = require('../../utils/onenet')

const fallbackDevices = [
  { id: '2559944541', name: '01', location: '默认设备', status: '未知', signal: '-', battery: '-' },
]

function mapPropertiesToMetrics(properties = []) {
  const dict = {}
  properties.forEach((item) => {
    dict[item.identifier] = item
  })

  return [
    {
      key: 'turd',
      title: '浊度',
      value: dict.turd?.value || '--',
      unit: 'NTU',
      level: '实时',
      desc: dict.turd?.name || '获取浊度',
      trend: dict.turd ? `更新时间 ${new Date(dict.turd.time).toLocaleTimeString()}` : '暂无数据',
    },
    {
      key: 'tds',
      title: 'TDS',
      value: dict.tds?.value || '--',
      unit: 'ppm',
      level: '实时',
      desc: dict.tds?.name || '获取电离度',
      trend: dict.tds ? `更新时间 ${new Date(dict.tds.time).toLocaleTimeString()}` : '暂无数据',
    },
    {
      key: 'temp',
      title: '水温',
      value: dict.temp?.value || '--',
      unit: '℃',
      level: '实时',
      desc: dict.temp?.name || '获取温度',
      trend: dict.temp ? `更新时间 ${new Date(dict.temp.time).toLocaleTimeString()}` : '暂无数据',
    },
    {
      key: 'ph',
      title: 'pH',
      value: dict.ph?.value || '--',
      unit: '',
      level: '实时',
      desc: dict.ph?.name || '获取酸碱度',
      trend: dict.ph ? `更新时间 ${new Date(dict.ph.time).toLocaleTimeString()}` : '暂无数据',
    },
  ]
}

Page({
  data: {
    projectName: '基于 ESP32-S3 的水质监测系统',
    subtitle: '毕业设计 · 小程序监控端',
    updateTime: '-',
    connectionHint: '启动后自动验证并同步设备，失败会自动跳转设备中心。',
    devices: fallbackDevices,
    currentDeviceIndex: 0,
    device: fallbackDevices[0],
    metrics: mapPropertiesToMetrics([]),
    bootstrapped: false,
    quickActions: [
      { key: 'deviceData', title: '设备总数据', tip: '进入当前设备实时总数据页' },
      { key: 'warning', title: '预警中心', tip: '阈值报警、消息开关' },
      { key: 'devices', title: '设备中心', tip: '配置鉴权并查看所有调试记录' },
      { key: 'refreshConfig', title: '刷新配置', tip: '设置自动刷新间隔等参数' },
    ],
  },

  async ensureAutoBootstrap() {
    if (this.data.bootstrapped) return
    try {
      const result = await oneNet.bootstrapAuto()
      const devices = result.devices.length ? result.devices : fallbackDevices
      this.setData({
        devices,
        currentDeviceIndex: 0,
        device: devices[0],
        bootstrapped: true,
      })
      await this.refreshCurrentDeviceOnce()
    } catch (err) {
      wx.showToast({ title: err.message || '自动验证失败，跳转设备中心', icon: 'none' })
      setTimeout(() => {
        wx.navigateTo({ url: '/pages/devices/devices' })
      }, 400)
    }
  },

  async refreshCurrentDeviceOnce() {
    const cfg = oneNet.getConfig()
    const device = this.data.device
    if (!device || !device.name) return

    try {
      const res = await oneNet.getDeviceLatestProperties(cfg.productId, device.name)
      this.setData({
        metrics: mapPropertiesToMetrics(Array.isArray(res.data) ? res.data : []),
        updateTime: new Date().toLocaleString(),
      })
    } catch (err) {
      // 首页不阻塞，只提示一次
      this.setData({ updateTime: `${new Date().toLocaleString()}（刷新失败）` })
    }
  },

  selectDevice(e) {
    const index = Number(e.currentTarget.dataset.index)
    const device = this.data.devices[index]
    this.setData({ currentDeviceIndex: index, device })
    this.refreshCurrentDeviceOnce()
  },

  goMetricDetail(e) {
    const { key, title, unit, value } = e.currentTarget.dataset
    wx.navigateTo({
      url: `/pages/metric/metric?key=${key}&title=${title}&unit=${unit || ''}&value=${value}&deviceName=${this.data.device.name}`,
    })
  },

  goQuickAction(e) {
    const { key } = e.currentTarget.dataset
    if (key === 'warning') {
      wx.navigateTo({ url: `/pages/warning/warning?deviceId=${this.data.device.id}` })
      return
    }
    if (key === 'devices') {
      wx.navigateTo({ url: '/pages/devices/devices' })
      return
    }
    if (key === 'deviceData') {
      wx.navigateTo({ url: `/pages/device-data/device-data?deviceName=${this.data.device.name}` })
      return
    }
    if (key === 'refreshConfig') {
      wx.navigateTo({ url: '/pages/refresh-config/refresh-config' })
      return
    }
  },

  onShow() {
    this.ensureAutoBootstrap()
  },

  onPullDownRefresh() {
    this.refreshCurrentDeviceOnce().finally(() => wx.stopPullDownRefresh())
  },
})
