Blockly.Blocks['iframe2_fustyles'] = {
  init: function() {
  this.appendValueInput('string_')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_SHOW);
  }
};

Blockly.Blocks['PageUrl'] = {
  init: function() {
  this.appendValueInput('string_')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_SHOW);
  }
};

Blockly.Blocks['PageWidth'] = {
  init: function() {
  this.appendValueInput('string_')
      .setCheck('Number')
      .appendField(Blockly.Msg.TEXT_SHOW);
  }
};

Blockly.Blocks['PageHeight'] = {
  init: function() {
  this.appendValueInput('string_')
      .setCheck('Number')
      .appendField(Blockly.Msg.TEXT_SHOW);
  }
};


