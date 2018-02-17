/**
 * @license Licensed under the Apache License, Version 2.0 (the "License"):
 *          http://www.apache.org/licenses/LICENSE-2.0
 */

/**
 * @fileoverview Code generator for the test 2 blocks.
 */
'use strict';

goog.provide('Blockly.Arduino.fustyles');

goog.require('Blockly.Arduino');


/** . */

Blockly.Arduino['fustyles_test'] = function(block) {
  var dropdown_value_red = block.getFieldValue('value_red');
  var dropdown_value_green = block.getFieldValue('value_green');
  var dropdown_value_blue = block.getFieldValue('value_blue');
  var value_pin_red = Blockly.Arduino.valueToCode(block, 'pin_red', Blockly.Arduino.ORDER_ATOMIC);
  var value_pin_green = Blockly.Arduino.valueToCode(block, 'pin_green', Blockly.Arduino.ORDER_ATOMIC);
  var value_pin_blue = Blockly.Arduino.valueToCode(block, 'pin_blue', Blockly.Arduino.ORDER_ATOMIC);
  var code = 'analogWrite('+dropdown_value_red+','+value_pin_red+');\n'+
			 'analogWrite('+dropdown_value_green+','+value_pin_green+');\n'+
			 'analogWrite('+dropdown_value_blue+','+value_pin_blue+');\n';
  return code;
};