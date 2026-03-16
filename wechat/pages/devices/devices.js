Page({
  data: {
    groupFilter: ['全部', '实验室', '户外'],
    currentGroup: '全部',
    devices: [
      { id: 'ESP32S3-WQ-001', name: '1号水箱', group: '实验室', status: '在线', latest: '1分钟前', gateway: 'OneNET-Channel-A' },
      { id: 'ESP32S3-WQ-002', name: '2号水箱', group: '实验室', status: '在线', latest: '3分钟前', gateway: 'OneNET-Channel-A' },
      { id: 'ESP32S3-WQ-003', name: '户外采样点', group: '户外', status: '离线', latest: '2小时前', gateway: 'OneNET-Channel-B' },
      { id: 'ESP32S3-WQ-004', name: '蓄水池边缘', group: '户外', status: '在线', latest: '5分钟前', gateway: 'OneNET-Channel-B' },
    ],
    shownDevices: [],
  },

  onLoad() {
    this.applyFilter('全部')
  },

  changeGroup(e) {
    const group = e.currentTarget.dataset.group
    this.applyFilter(group)
  },

  applyFilter(group) {
    const shownDevices = this.data.devices.filter((item) => group === '全部' || item.group === group)
    this.setData({
      currentGroup: group,
      shownDevices,
    })
  },

  bindDevice(e) {
    const { id } = e.currentTarget.dataset
    wx.showToast({ title: `设备 ${id} 已加入（演示）`, icon: 'none' })
  },
})
