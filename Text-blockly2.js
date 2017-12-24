+(function (window, $) {

  'use strict';

  var blocklyText = function (text) {
    if ($('.text-wrapper').length < 1) {
      this.wrapper = $('<div/>').addClass('text-wrapper')
      $('body').append(this.wrapper);
    }
    this.paragraph = $('<p/>').appendTo('.text-wrapper');
    if (text) {
      this.paragraph.text(text);
    }
  }

  blocklyText.prototype.setText = function (text) {
    this.paragraph.text(text);
    return this;
  };

  window.blocklyText = blocklyText;

}(window, window.jQuery));