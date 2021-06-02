let handlers = {};
let root = document.body.querySelector(".root");
let content = document.body.querySelector(".content");

const addEvent = (el, name, fn) => {
    el.addEventListener(name, fn);
};

const clickHandler = (btn) => {
    let type = btn.getAttribute("data-type");
    let fn = handlers[type];
    fn && fn();
};
const example = (type, fn) => {
    handlers[type] = fn;
};
const addSpan = (btn)=>{
    let span = document.createElement('span');
    span.innerText = btn.innerText
    btn.innerText = ''
    btn.appendChild(span)
}
addEvent(document, "DOMContentLoaded", (_) => {
    let arr = document.body.querySelectorAll(".btn");

    Array.from(arr).forEach((btn) => {
        addSpan(btn)
        addEvent(btn, "click", (_) => {
            clickHandler(btn);
        });
        content.appendChild(btn);
    });
});

