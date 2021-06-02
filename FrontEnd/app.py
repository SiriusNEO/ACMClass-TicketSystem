from flask import Flask, request, render_template, jsonify, make_response
from core import Courier

app = Flask(__name__)

courier = Courier('./code')

'''
def destruct(): #still debuging
    print("清除cookies")
    resp = make_response(render_template('./index.html', is_login=(False, "Logout")))
    resp.delete_cookie('name')
    resp.delete_cookie('id')
    resp.delete_cookie('passwd')
'''

@app.route('/', methods=['GET', 'POST'])
def index():
    cur_user_info = request.cookies
    if cur_user_info:
        print("检测到cookies，登录")
        ret = courier.login(cur_user_info['id'], cur_user_info['passwd'])
        if ret[1] == 'user_not_found': #用户不存在，cookies作废
            resp = make_response(render_template('./index.html', is_login=(False, "")))
            resp.delete_cookie('name')
            resp.delete_cookie('id')
            resp.delete_cookie('passwd')
            return resp
        return render_template('./index.html', is_login=(True, cur_user_info['name']))
    return render_template('./index.html', is_login=(False, ""))

@app.route('/cmd', methods=['POST'])
def cmd():
    form = request.form.to_dict()
    return render_template('./result.html', query_name="控制台命令",
                           hint1='\nTicket System Terminal,\nA simple terminal for testing this stupid system.\nACM Class, PingPong Doubles Smash Locomotive, 2021.\n\n<<< '+form['cmd_input']+'\n\n>>> ',
                           hint2='\n ...\n Complete.Please check the output.',
                           output='\n' + courier.cmd(form['cmd_input']))


@app.route('/login', methods=['POST'])
def login():
    form = request.form.to_dict()
    ret = courier.login(form['login_userid_input'], form['login_passwd_input'])
    if ret[0] == True:
        resp = make_response(render_template('./result.html', query_name="密码验证结果", output=ret))
        resp.set_cookie('name', ret[1])
        resp.set_cookie('id', ret[2])
        resp.set_cookie('passwd', ret[3])
        return resp

    return render_template('./result.html', query_name="密码验证结果", output=ret)

@app.route('/logout')
def logout():
    print("success")
    cur_user_info = request.cookies
    if cur_user_info:
        cur_id = cur_user_info['id']
    else:
        cur_id = "404_not_logged"
    courier.logout(cur_id)
    resp = make_response(render_template('./index.html', is_login=(False, "Logout")))
    resp.delete_cookie('name')
    resp.delete_cookie('id')
    resp.delete_cookie('passwd')
    return resp

@app.route('/add_user', methods=['POST'])
def add_user():
    cur_user_info = request.cookies
    if cur_user_info:
        cur_id = cur_user_info['id']
    else:
        cur_id = "404_not_logged"
    form = request.form.to_dict()
    return render_template('./result.html', query_name="用户添加结果", output=courier.add_user(cur_id, form['add_userid'], form['add_passwd'],
                                                         form['add_name'], form['add_email'], form['add_privilege']))

@app.route('/register', methods=['POST'])
def register():
    form = request.form.to_dict()
    courier.login("root", "root")
    output = courier.add_user("root", form['register_userid'], form['register_passwd'],
                                                   form['register_name'], form['register_email'], "1")
    courier.logout("root")
    return render_template('./result.html', query_name="注册结果", output=output)

@app.route('/query_profile', methods=['POST'])
def query_profile():
    cur_user_info = request.cookies
    if cur_user_info:
        cur_id = cur_user_info['id']
    else:
        cur_id = "404_not_logged"
    form = request.form.to_dict()
    return render_template('./result.html', query_name="用户查询结果", output=courier.query_profile(cur_id, form['query_id']))

@app.route('/modify_profile', methods=['POST'])
def modify_profile():
    cur_user_info = request.cookies
    if cur_user_info:
        cur_id = cur_user_info['id']
    else:
        cur_id = "404_not_logged"
    form = request.form.to_dict()
    return render_template('./result.html', query_name="用户修改结果", output=courier.modify_profile(cur_id, form['modify_userid'], form['modify_passwd'],
                                                         form['modify_name'], form['modify_email'], form['modify_privilege']))

@app.route('/query_ticket', methods=['POST'])
def query_ticket():
    form = request.form.to_dict()
    date_str = form['ticket_date'][5:]
    if form['ticket_priority'] == '越快越好':
        pri_str = 'time'
    elif form['ticket_priority'] == '实惠优先':
        pri_str = 'cost'
    return render_template('./result.html', query_name="车票查询结果", output=courier.query_ticket(form['ticket_s'], form['ticket_t'], date_str, pri_str))

@app.route('/query_transfer', methods=['POST'])
def query_transfer():
    form = request.form.to_dict()
    date_str = form['transfer_date'][5:]
    if form['transfer_priority'] == '越快越好':
        pri_str = 'time'
    elif form['transfer_priority'] == '实惠优先':
        pri_str = 'cost'
    return render_template('./result.html', query_name="换乘查询结果",
                           output=courier.query_transfer(form['transfer_s'], form['transfer_t'], date_str, pri_str))

@app.route('/buy_ticket', methods=['POST'])
def buy_ticket():
    cur_user_info = request.cookies
    if cur_user_info:
        cur_id = cur_user_info['id']
    else:
        cur_id = "404_not_logged"
    form = request.form.to_dict()
    print(form)
    date_str = form['buy_date'][5:]
    if form['buy_q'] == '不接受（余票不足将直接购买失败）':
        pri_str = 'false'
    elif form['buy_q'] == '接受（余票不足进入候补队列，有票时立即购买）':
        pri_str = 'true'
    return render_template('./result.html', query_name="购票结果",
                           output=courier.buy_ticket(cur_id, form['buy_trainid'], date_str, form['buy_s'], form['buy_t'], form['buy_n'], pri_str))

@app.route('/query_order', methods=['POST'])
def query_order():
    cur_user_info = request.cookies
    if cur_user_info:
        cur_id = cur_user_info['id']
    else:
        cur_id = "404_not_logged"
    return render_template('./result.html', query_name="订单查询结果",
                           output=courier.query_order(cur_id))

@app.route('/refund_ticket', methods=['POST'])
def refund_ticket():
    cur_user_info = request.cookies
    if cur_user_info:
        cur_id = cur_user_info['id']
    else:
        cur_id = "404_not_logged"
    form = request.form.to_dict()
    return render_template('./result.html', query_name="退票结果", output=courier.refund_ticket(cur_id, form['refund_n']))

@app.route('/add_train', methods=['POST'])
def add_train():
    form = request.form.to_dict()
    station_num = int(form['station_num'])
    train_str = ''
    train_str += ' -i ' + form['add_trainid']
    train_str += ' -n ' + form['station_num']
    train_str += ' -y ' + form['add_type']
    train_str += ' -m ' + form['add_seatnum']
    train_str += ' -d ' + form['add_startdate'][5:] + '|' + form['add_enddate'][5:]
    train_str += ' -x ' + form['add_starttime']
    train_str += ' -s ' + form['station_name1']
    for i in range(2, station_num+1):
        train_str += '|' + form['station_name'+str(i)]
    train_str += ' -p ' + form['price1']
    for i in range(2, station_num):
        train_str += '|' + form['price'+str(i)]
    train_str += ' -t ' + form['travel_time1'];
    for i in range(2, station_num):
        train_str += '|' + form['travel_time'+str(i)]
    if station_num > 2:
        train_str += ' -o ' + form['stopover_time2']
        for i in range(3, station_num):
            train_str += '|' + form['stopover_time' + str(i)]
    return render_template('./result.html', query_name="火车添加结果", output=courier.add_train(train_str))

@app.route('/query_train', methods=['POST'])
def query_train():
    form = request.form.to_dict()
    return render_template('./result.html', query_name="火车查询结果", output=courier.query_train(form['query_trainid'], form['query_date'][5:]))

@app.route('/release_train', methods=['POST'])
def release_train():
    print("发行")
    form = request.form.to_dict()
    return render_template('./result.html', query_name="火车发行结果", output=courier.release_train(form['release_delete_tid']))

@app.route('/delete_train', methods=['POST'])
def delete_train():
    form = request.form.to_dict()
    return render_template('./result.html', query_name="火车删除结果", output=courier.delete_train(form['release_delete_tid']))

courier.add_user("root", "root", "root", "root", "root", "10")

if __name__ == '__main__':
    app.run(port=5000, debug=True)