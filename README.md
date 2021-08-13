# Ticket System 2020

ACM Class 2020, a train ticket manage system based on B+ Tree data structure.



## 关于我们

### We are [@PingPong Doubles Smash Locomotive](https://github.com/SiriusNEO/ACMClass-TicketSystem)

> 希望我们的代码能像乒乓球的扣杀一样又快又狠


|  组员   | 分工  |
|  ----  | ----  |
| 林超凡 [@SiriusNEO](https://github.com/siriusneo)  | 后端逻辑，前端（待定？） |
| 王崑运 [@wangky-ac](https://github.com/wangky-ac) | B+树，缓存 |



### 导航

想了解本作业的需求，请查看 [作业需求文档](Document/AssignmentRequirement)

想了解开发情况，请查看 [开发文档](Document/Development/开发文档.pdf)

部署或使用此系统，请查看 [用户手册](FrontEnd/static/doc/help.pdf)



## 环境需求与部署方式

后端部分 `BackEnd/` 为纯 C++ 编写，支持 C++11 的编译器编译均可。

前端部分用到了 `python Flask` 框架。Python 环境详见 `FrontEnd/requirements.txt`

**简易部署教学**

1. `git clone https://github.com/SiriusNEO/ACMClass-TicketSystem.git`
2. （后端）`cd BackEnd/ && cmake . && make && ./code`
3. （前端）`cd FrontEnd/ && python3 app.py`

可以使用系统预置的数据库 `FrontEnd/datainit` 进行测试。

详情请见 [用户手册](FrontEnd/static/doc/help.pdf)。

