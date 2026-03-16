const devices = [
  {
    id: 'ESP32S3-WQ-001',
    name: '1号水箱',
    location: '实验室 A-203',
    status: '在线',
    signal: '良好',
    battery: 86,
  },
  {
    id: 'ESP32S3-WQ-002',
    name: '2号水箱',
    location: '实验室 A-205',
    status: '在线',
    signal: '中等',
    battery: 74,
  },
  {
    id: 'ESP32S3-WQ-003',
    name: '户外采样点',
    location: '南门景观池',
    status: '离线',
    signal: '弱',
    battery: 41,
  },
]

Page({
  data: {
    projectName: '基于 ESP32-S3 的水质监测系统',
    subtitle: '毕业设计 · 小程序监控端（前端演示版）',
    updateTime: '2026-03-16 12:00',
    connectionHint: '当前为静态演示数据，后续将通过 OneNET 接入实时设备数据。',
    devices,
    currentDeviceIndex: 0,
    device: devices[0],
    metrics: [
      {
        key: 'turbidity',
        title: '浊度',
        value: '12',
        unit: 'NTU',
        level: '优',
        desc: '水体清澈，悬浮物较少',
        trend: '↘ 较上次下降 2 NTU',
      },
      {
        key: 'tds',
        title: 'TDS',
        value: '238',
        unit: 'ppm',
        level: '正常',
        desc: '总溶解固体处于可接受范围',
        trend: '→ 与上次持平',
      },
      {
        key: 'temp',
        title: '水温',
        value: '24.6',
        unit: '℃',
        level: '适宜',
        desc: '温度适合常规养殖与检测环境',
        trend: '↗ 较上次上升 0.4℃',
      },
      {
        key: 'ph',
        title: 'pH',
        value: '7.2',
        unit: '',
        level: '中性',
        desc: '酸碱度接近中性',
        trend: '→ 波动很小',
      },
    ],
    quickActions: [
      { key: 'warning', title: '预警中心', tip: '阈值报警、短信/订阅消息开关' },
      { key: 'devices', title: '设备管理', tip: '多设备接入、在线状态与分组' },
      { key: 'history', title: '历史记录', tip: '后续对接云端时间序列回放' },
    ],
  },

  selectDevice(e) {
    const index = Number(e.currentTarget.dataset.index)
    const device = this.data.devices[index]
    this.setData({
      currentDeviceIndex: index,
      device,
    })
  },

  goMetricDetail(e) {
    const { key, title, unit, value } = e.currentTarget.dataset
    wx.navigateTo({
      url: `/pages/metric/metric?key=${key}&title=${title}&unit=${unit || ''}&value=${value}`,
    })
  },

  goQuickAction(e) {
    const { key } = e.currentTarget.dataset
    if (key === 'warning') {
      wx.navigateTo({ url: '/pages/warning/warning' })
      return
    }
    if (key === 'devices') {
      wx.navigateTo({ url: '/pages/devices/devices' })
      return
    }

    wx.showToast({
      title: '该功能将随 OneNET 联调后开放',
      icon: 'none',
    })
  },

  onPullDownRefresh() {
    wx.showToast({
      title: '演示模式：暂无实时刷新',
      icon: 'none',
    })

    setTimeout(() => {
      wx.stopPullDownRefresh()
    }, 400)
  },
})
