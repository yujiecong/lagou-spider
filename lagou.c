import requests
import time
import threading
import xlwt
import xlrd
from pyecharts import Pie, Bar
import os

class spider():
	def __init__(self):
		self.headers_boss = {
			"User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.119 Safari/537.36",
		}
		r=requests.get(url="https://www.zhipin.com/job_detail/?query=%E5%B5%8C%E5%85%A5%E5%BC%8F&city=101280100&industry=&position=",headers=self.headers_boss)
		# print(r.text)
		self.search = 'linux嵌入式'
		self.city = '广州'
		self.all_values = []
		self.all_keys = []
		# https://www.zhipin.com/job_detail/?query=%E5%B5%8C%E5%85%A5%E5%BC%8F&city=101280100&industry=&position=
		self.url_LAGOU = "https://www.lagou.com/jobs/list_" + self.search
		self.headers_LAGOU = {
			"User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.119 Safari/537.36",
			"Referer": "https://www.lagou.com/jobs/list_" + ''+ "?&px=default&city=%e6%b7%b1%e5%9c%b3",
			"Content-Type": "application/x-www-form-urlencoded;charset = UTF-8"}
		self.params_LAGOU = {'px': 'default', 'city': self.city}
		self.response_LAGOU = requests.get(url=self.url_LAGOU, headers=self.headers_LAGOU, params=self.params_LAGOU)
		self.start()
	def next_page(self, suffix):
		if suffix >= 2:
			status = False
		else:
			status = True
		self.data_LAGOU = {'first': status, 'pn': suffix, 'kd': self.search}
		self.response_cookies = self.response_LAGOU.cookies
		self.json_url = "https://www.lagou.com/jobs/positionAjax.json"
		self.json_params = {'px': 'default', 'city': self.city, 'needAddtionalResult': 'false'}
		try:
			self.response_json = requests.post(url=self.json_url, cookies=self.response_cookies,
			                                   params=self.json_params, headers=self.headers_LAGOU,
			                                   data=self.data_LAGOU)
			position = dict(self.response_json.json())['content']['positionResult']['result']
			self.all_keys = list(position[0].keys())
			for p in position:
				self.all_values.append(p)
		except:
			time.sleep(.5)
			print('访问太快了，放慢点')
	
	def start(self):
		self.page = 20
		self.threads = []
		for _ in range(1, self.page):
			self.threads.append(threading.Thread(target=self.next_page, args=[_]))
		for t in self.threads:
			t.start()
		while (threading.active_count() != 1):
			time.sleep(.1)
		self.data_Analysis()
		self.draw_charts()  # 画图
	
	def data_Analysis(self):
		workbook = xlwt.Workbook(encoding='utf-8')
		# 创建表,第二参数用于确认同一个cell单元是否可以重设值
		worksheet = workbook.add_sheet(self.search, cell_overwrite_ok=True)
		for i in range(len(self.all_keys)):
			worksheet.write(0, i, self.all_keys[i].lower())
		for vi in range(len(self.all_values)):  # y轴
			for ki, keys in enumerate(self.all_keys):  # x轴
				worksheet.write(vi + 1, ki, self.all_values[vi][keys])
		if os.path.exists('%s%s表格.xls'% (self.city, self.search)):
			os.remove('%s%s表格.xls'% (self.city, self.search))
		workbook.save('%s%s表格.xls' % (self.city, self.search))
	
	def draw_charts(self):
		data = xlrd.open_workbook('%s%s表格.xls' % (self.city, self.search))
		table = data.sheets()[0]
		ops = ['district', 'education', 'salary', 'companyshortname']
		for op in ops:
			INDEX = table.row_values(0).index(op)
			statistics = {}
			for dis in table.col_values(INDEX)[1:]:
				if dis not in statistics:
					statistics[dis] = 1
				else:
					statistics[dis] += 1
			c = Pie(
				self.city + "地区%s %s 图表" % (self.search, op),
				title_pos='center'
			)
			keys = list(statistics.keys())
			values = list(statistics.values())
			top10_values = []
			top10_keys = []
			if op == 'salary' or op == 'companyshortname':
				for _ in range(10):
					index = values.index(max(values))
					top10_values.append(values[index])
					top10_keys.append(keys[index])
					values.pop(index)
					keys.pop(index)
				keys = top10_keys
				values = top10_values
			
			c.add(
				"",
				keys,
				values,
				is_label_show=True,
				legend_orient="vertical",
				legend_pos="left",
			)
			c.render(self.city + "%s %s charts.html" % (self.search, op))
	
	def draw_bar(self):
		city = ['深圳', '广州', '佛山']
		ops = ['education', 'salary']
		b = Bar(
			f"{city}对比",
		)
		for op in ops:
			for c in city:
				data = xlrd.open_workbook('%s%s表格.xls' % (c, self.search))
				table = data.sheets()[0]
				INDEX = table.row_values(0).index(op)
				statistics = {}
				for dis in table.col_values(INDEX)[1:]:
					if dis not in statistics:
						statistics[dis] = 0
					else:
						statistics[dis] += 1
				keys = list(statistics.keys())
				values = list(statistics.values())
				top10_values = []
				top10_keys = []
				if op == 'salary':
					for _ in range(10):
						index = values.index(max(values))
						top10_values.append(values[index])
						top10_keys.append(keys[index])
						values.pop(index)
						keys.pop(index)
					keys = top10_keys
					print(keys)
					values = top10_values
				
				b.add(
					c,
					keys,
					values,
					is_label_show=True, is_datazoom_show=True
				)
			b.render(f"{city}的%s对比 .html" % op)


spider()
