function userid_validator(str) {return validator(str, '^[a-zA-Z][a-zA-Z0-9_]{0,19}$');}
function passwd_validator(str) {return validator(str, '^[a-zA-Z0-9_]{4,30}$');}
function name_validator(str) {return validator(str, '^[\u4e00-\u9fa5]{2,5}$');}
function mailAddr_validator(str) {return validator(str, '^[a-zA-Z0-9@.]{0,30}$');}
function priviledge_validator(cnt) {
    return parseInt(cnt) >= 0 && parseInt(cnt) <= 10;
}

function trainid_validator(str) {return validator(str, '^[a-zA-Z][a-zA-Z0-9_]{0,19}$')}
function stationNum_validator(cnt) {
    return parseInt(cnt) >= 2 && parseInt(cnt) <= 100;
}
function station_validator(str) {
    return validator(str, '^[\u4e00-\u9fa5]{1,10}$');
}
function seatNum_validator(cnt) {
    return parseInt(cnt) >= 0 && parseInt(cnt) <= 100000;
}
function travelTime_validator(cnt) {
    return parseInt(cnt) >= 0 && parseInt(cnt) <= 10000;
}
function stopoverTime_validator(cnt) {
    return parseInt(cnt) >= 0 && parseInt(cnt) <= 10000;
}
function price_validator(cnt) {
    return parseInt(cnt) >= 0 && parseInt(cnt) <= 10000;
}
function type_validator(str) {
    return validator(str, '^[A-Z]{1}$');
}

function validator(str, regex) {
    return str.match(regex) != null;
}

function hint(validator_func, str, id) {
    if (validator_func(str)) {
       xtip.msg('输入合法',{icon:'s'});
       return true;
    }
    else {
         xtip.msg('输入不合法',{icon:'e'});
         document.getElementById(id).value = '';
         return false;
    }
}