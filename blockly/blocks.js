Blockly.Blocks['showimage_france'] = {
  init: function() {
    this.appendValueInput("ImageUrl")
        .setCheck("String")
        .appendField("ImageUrl");
    this.appendValueInput("ImageWidth")
        .setCheck("Number")
        .appendField("ImageWidth");
    this.appendValueInput("ImageHeight")
        .setCheck("Number")
        .appendField("ImageHeight");
    this.setOutput(true, null);
    this.setColour(180);
 this.setTooltip("Hello World");
 this.setHelpUrl("");
  }
};