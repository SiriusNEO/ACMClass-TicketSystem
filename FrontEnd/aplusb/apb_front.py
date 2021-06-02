from flask import Flask, request, render_template
from apb_core import Calculator

app = Flask(__name__)
calculator = Calculator('./apb')

@app.route('/')
def index():
    return render_template('./indexpage.html')

@app.route('/calc', methods=['GET', 'POST'])
def front():
    if request.method == 'GET':
        return render_template('./inputpage.html')

    form = request.form.to_dict()
    a = int(form['a'])
    b = int(form['b'])
    print(a, b)
    return "The Result is " + calculator.add(a, b)

if __name__ == '__main__':
    app.run(port=5000)