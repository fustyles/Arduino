Blockly.JavaScript['iframe2_fustyles'] = function (block) {
  var value_pageurl = Blockly.JavaScript.valueToCode(block, 'PageUrl', Blockly.JavaScript.ORDER_ATOMIC);
  var value_pagewidth = Blockly.JavaScript.valueToCode(block, 'PageWidth', Blockly.JavaScript.ORDER_ATOMIC);
  var value_pagheight = Blockly.JavaScript.valueToCode(block, 'PagHeight', Blockly.JavaScript.ORDER_ATOMIC);

  var code = ' document.getElementById("demo-area-01-show").innerHTML = "<iframe src='+ value_pageurl  +' width=' + value_pagewidth + ' height=' + value_pagheight + '>"';
  return [code, Blockly.JavaScript.ORDER_NONE];
};