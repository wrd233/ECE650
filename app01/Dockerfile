#使用的基础镜像
FROM python:3.6
#设置工作目录
WORKDIR /app
#复制requirements.txt
COPY requirements.txt requirements.txt

# 安装pip并升级到最新版本
# RUN python3 -m pip install --upgrade pip

#安装依赖包
RUN pip install -r requirements.txt

# 安装 PostgreSQL 服务器和客户端
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        postgresql \
        postgresql-contrib \
    && rm -rf /var/lib/apt/lists/*

# 切换到 postgres 用户
USER postgres

# 修改 postgres 用户的密码
RUN /etc/init.d/postgresql start \
    && psql --command "ALTER USER postgres WITH PASSWORD 'passw0rd';"

# 创建 AA_BBALL 数据库
RUN /etc/init.d/postgresql start \
    && psql --command "CREATE DATABASE ACC_BBALL;"

# 切换回根用户
USER root

# 暴露 PostgreSQL 服务器的默认端口
EXPOSE 5432

#复制当前目录下的内容到docker中
COPY . .

CMD ["bash"]