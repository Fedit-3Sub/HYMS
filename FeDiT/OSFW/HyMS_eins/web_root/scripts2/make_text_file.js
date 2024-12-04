let _text_file_url = null;
function make_text_file(content, filename) {
    var data = new Blob([content], {type: 'text/plain'});
    if (_text_file_url !== null) {
        window.URL.revokeObjectURL(_text_file_url);
    }
    _text_file_url = window.URL.createObjectURL(data);

    const a = document.createElement('a');
    document.body.appendChild(a);
    a.href = _text_file_url;
    a.download = filename;
    a.click();
    setTimeout(() => {
      window.URL.revokeObjectURL(_text_file_url);
      document.body.removeChild(a);
    }, 0);
    return _text_file_url;
};

