var node_style = new Array("success", "danger", "prime", "warning");
var node_style_num = 4;

var node_total = 2;
var node_pre = 2;

append_node(2);

function append_node(cnt) {
    var new_node;
    if (cnt != node_total) {
        new_node = '\
        <div class="oneNode" id="node_'+cnt+'">\
            <div class="check check-'+node_style[(cnt-1)%4]+'">S'+cnt+'</div>\
            <div class="tag-boder"><div class="tag"></div></div>\
            <div class="NodeDetail">\
                <div class="NodeDetail-content">\
                        &emsp;<i class="icon fa fa-map-marker"></i>\
                        <input type="text" id="station_name'+cnt+'" onchange="hint(station_validator, $(this).val(), \'station_name'+cnt+'\')" name="station_name'+cnt+'" class="form-control-static" placeholder="站名" required="required" style="font-size: small; width: 180px; border-color: Blue; text-indent: 10px">&emsp;&emsp;\
                        <br><br>\
                        &emsp;<input type="number" id="travel_time'+cnt+'" onchange="hint(travelTime_validator, $(this).val(), \'travel_time'+cnt+'\')" name="travel_time'+cnt+'" class="form-control-static" placeholder="到下站历时" style="font-size: small; text-indent: 10px; width: 160px" required="required"  />\
                        &emsp;&emsp;<input type="number" id="stopover_time'+cnt+'" onchange="hint(stopoverTime_validator, $(this).val(), \'stopover_time'+cnt+'\')" name="stopover_time'+cnt+'" class="form-control-static" placeholder="停靠时长" style="font-size: small; text-indent: 10px; width: 160px" required="required"  />\
                        &emsp;&emsp;<input type="number" id="price'+cnt+'" onchange="hint(price_validator, $(this).val(), \'price'+cnt+'\')" name="price'+cnt+'" class="form-control-static" placeholder="到下站票价" style="font-size: small; text-indent: 10px; width: 160px" required="required"  />\
                </div>\
            </div>\
        </div>';
    }
    else {
        new_node = '\
        <div class="oneNode" id="node_'+cnt+'">\
            <div class="check check-'+node_style[(cnt-1)%4]+'">终</div>\
            <div class="tag-boder"><div class="tag"></div></div>\
            <div class="NodeDetail">\
                <div class="NodeDetail-content">\
                        &emsp;<i class="icon fa fa-map-marker"></i>\
                        <input type="text" id="station_name'+cnt+'" name="station_name'+cnt+'" onchange="hint(station_validator, $(this).val(), \'station_name'+cnt+'\')" class="form-control-static" placeholder="站名" required="required" style="font-size: small; width: 180px; border-color: Blue; text-indent: 10px">&emsp;&emsp;\
                        <br><br>\
                        &emsp;<input type="number" id="travel_time'+cnt+'" name="travel_time'+cnt+'" onchange="hint(station_validator, $(this).val(), \'travel_time'+cnt+'\')" class="form-control-static" disabled="true" placeholder="终点站无下一站" style="font-size: small; text-indent: 10px; width: 160px" required="required"  />\
                        &emsp;&emsp;<input type="number" id="stopover_time'+cnt+'" name="stopover_time'+cnt+'" onchange="hint(station_validator, $(this).val(), \'stopover_time'+cnt+'\')" class="form-control-static" disabled="true" placeholder="终点站无停靠时长" style="font-size: small; text-indent: 10px; width: 160px" required="required"  />\
                        &emsp;&emsp;<input type="number" id="price'+cnt+'" onchange="hint(station_validator, $(this).val(), \'price'+cnt+'\')" name="price'+cnt+'" onchange="hint(station_validator, $(this).val(), \'price'+cnt+'\')" class="form-control-static" disabled="true" placeholder="终点站无下一站" style="font-size: small; text-indent: 10px; width: 160px" required="required"  />\
                </div>\
            </div>\
        </div>';
    }
    $("#node_set").append(new_node);
}

function remove_node(cnt) {
    $("#node_"+cnt).remove();
}

function modify_len(len) {
    let i;
    if (node_pre == len) return;
    node_pre = node_total;
    remove_node(node_total);
    node_total = parseInt(len);
    if (node_pre < node_total) {
        for (i = node_pre; i <= node_total; ++i) {
            append_node(i);
        }
    }
    else if (node_pre > node_total) {
        for (i = node_pre; i >= node_total; --i) {
            remove_node(i);
        }
        append_node(node_total);
    }
}