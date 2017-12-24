Blockly.Blocks['iframe2_fustyles'] = {
  init: function() {
  this.appendValueInput('value_')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_MAIN);
  }
};

Blockly.Blocks['PageUrl'] = {
  init: function() {
  this.appendValueInput('value_')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_URL);
  }
};

Blockly.Blocks['PageWidth'] = {
  init: function() {
  this.appendValueInput('value_')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_WIDTH);
  }
};

Blockly.Blocks['PageHeight'] = {
  init: function() {
  this.appendValueInput('value_')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_HEIGHT);
  }
};


