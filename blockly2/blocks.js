Blockly.Blocks['iframe2_fustyles'] = {
  init: function() {
  this.appendValueInput('iframe2_fustyles')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_MAIN);
  }
};

Blockly.Blocks['PageUrl'] = {
  init: function() {
  this.appendValueInput('PageUrl')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_URL);
  }
};

Blockly.Blocks['PageWidth'] = {
  init: function() {
  this.appendValueInput('PageWidth')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_WIDTH);
  }
};

Blockly.Blocks['PageHeight'] = {
  init: function() {
  this.appendValueInput('PageHeight')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_HEIGHT);
  }
};


