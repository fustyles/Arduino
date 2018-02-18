/**
 * @license Licensed under the Apache License, Version 2.0 (the "License"):
 *          http://www.apache.org/licenses/LICENSE-2.0
 */

/**
 * @fileoverview Ardublockly JavaScript for the Blockly resources and bindings.
 */
'use strict';

goog.provide('Blockly.Blocks.fustyles');

goog.require('Blockly.Blocks');

/* Ardublockly logo block */
Blockly.Blocks['fustyles_test'] = {
  init: function() {
    this.appendValueInput("pin_red")
        .setCheck("Number")
		.setAlign(Blockly.ALIGN_RIGHT)
		.appendField(Blockly.Msg.BLOCKS_FUSTYLES_TITLE)
        .appendField(Blockly.Msg.BLOCKS_FUSTYLES_RED_TITLE)
        .appendField(new Blockly.FieldDropdown([["3","3"], ["5","5"], ["6","6"], ["9","9"], ["10","10"], ["11","11"]]), "value_red")
        .appendField(Blockly.Msg.BLOCKS_FUSTYLES_RED_VALUE);
    this.appendValueInput("pin_green")
        .setCheck("Number")
		.setAlign(Blockly.ALIGN_RIGHT)
		.appendField(Blockly.Msg.BLOCKS_FUSTYLES_GREEN_TITLE)
        .appendField(new Blockly.FieldDropdown([["3","3"], ["5","5"], ["6","6"], ["9","9"], ["10","10"], ["11","11"]]), "value_green")
        .appendField(Blockly.Msg.BLOCKS_FUSTYLES_GREEN_VALUE);
    this.appendValueInput("pin_blue")
        .setCheck("Number")
		.setAlign(Blockly.ALIGN_RIGHT)
		.appendField(Blockly.Msg.BLOCKS_FUSTYLES_BLUE_TITLE)
        .appendField(new Blockly.FieldDropdown([["3","3"], ["5","5"], ["6","6"], ["9","9"], ["10","10"], ["11","11"]]), "value_blue")
		.appendField(Blockly.Msg.BLOCKS_FUSTYLES_BLUE_VALUE);
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour(345);
 this.setTooltip("");
 this.setHelpUrl("");
  }
};