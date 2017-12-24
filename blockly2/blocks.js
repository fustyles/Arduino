Blockly.Blocks['iframe2_fustyles'] = {
  init: function() {
  this.appendValueInput('string_')
      .setCheck('String')
      .appendField(Blockly.Msg.TEXT_SHOW);
  this.setOutput(false, null);
  this.setTooltip('');
  this.setColour(Blockly.Blocks.colour.HUE);
  this.setHelpUrl('');
  }
};
