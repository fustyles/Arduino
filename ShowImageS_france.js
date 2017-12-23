Blockly.JavaScript['showimage_france'] = function(block) {
  var value_imageurl = Blockly.JavaScript.valueToCode(block, 'ImageUrl', Blockly.JavaScript.ORDER_ATOMIC);
  var value_imagewidth = Blockly.JavaScript.valueToCode(block, 'ImageWidth', Blockly.JavaScript.ORDER_ATOMIC);
  var value_imageheight = Blockly.JavaScript.valueToCode(block, 'ImageHeight', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = '...';
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_NONE];
};