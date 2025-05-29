
# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import smtplib  
from email.mime.multipart import MIMEMultipart  
from email.mime.base import MIMEBase  
from email.mime.text import MIMEText  
from email import encoders  
import os
import sys
  
# SMTP服务器配置
smtp_server = 'smtp.x.com'
smtp_port = 25
sender_id = 'test'
sender_email= ''
sender_password = ''  
receiver_email = ['1@x.com', '2@x.com', '3@x.com', '4@x.com']
#receiver_email = ['1@x.com']  
# 定义包含文件夹路径的列表  
sanitizers = sys.argv[3:] 
  
# 创建一个MIMEMultipart对象  
message = MIMEMultipart()  
message['From'] = sender_email  
message['To'] = ', '.join(receiver_email)
message['Subject'] = sys.argv[1]

# 创建表头
html_table_rows = []
html_table_rows.append('<tr><th width="100">测试类型</th><th>测试结果</th></tr>')

# 遍历文件夹列表
for sanitizer in sanitizers:
    # 构建文件路径
    file_path = os.path.join('./', sanitizer+'.result')

    # 检查文件是否存在
    if os.path.exists(file_path):
        # 读取文件内容
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()

        # 创建一个表格行，包含测试类型和结果
        html_table_rows.append(f'<tr><td>{sanitizer}</td><td>{content.replace(os.linesep, "<br>")}</td></tr>')

# 将所有的表格行组合成一个完整的表格
html_table = '<table border="1">' + ''.join(html_table_rows) + '</table>'
  
# HTML邮件内容
html_content = f"""
<html>
<head></head>
<body>
    <p><tr><td><b>{sys.argv[2]}</b></td></tr></p>
    <ul>
        {html_table}
    </ul>
</body>
</html>
"""

# 设置邮件正文  
message.attach(MIMEText(html_content, 'html', 'utf-8'))  

# 发送邮件  
try:
    server = smtplib.SMTP(smtp_server, smtp_port)
    server.starttls()  # 如果SMTP服务器支持TLS
    server.login(sender_id, sender_password)
    server.sendmail(sender_email, receiver_email, message.as_string())
    print("send result email successfully!")
except Exception as e:
    print(f"send result email failed: {e}")
finally:
    server.quit()
