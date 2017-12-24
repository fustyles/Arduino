Blockly.JavaScript['iframe2_fustyles'] = function (block) {
  var value_pageurl = Blockly.JavaScript.valueToCode(block, 'Url', Blockly.JavaScript.ORDER_ATOMIC);
  var value_pagewidth = Blockly.JavaScript.valueToCode(block, 'Width', Blockly.JavaScript.ORDER_ATOMIC);
  var value_pageheight = Blockly.JavaScript.valueToCode(block, 'Height', Blockly.JavaScript.ORDER_ATOMIC);

  var code = ' document.getElementById("demo-area-01-show").innerHTML = "<iframe src=' + value_pageurl  + ' width=' + value_pagewidth + ' height=' + value_pageheight + '>";\n';
  return code;
};
