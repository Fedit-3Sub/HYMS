function getMonacoDoc(atomic_name, param){

    // var repParam = param.replaceAll("\n", "\\n").replaceAll("'","\\'").replaceAll('"','\\"');

    //  repParam = repParam.replaceAll("\r\n", "\\n").replaceAll("'","\\'").replaceAll('"','\\"');
    
    

    var   repParam = param.replaceAll("\n", "\\n").replaceAll("\r", "").replaceAll("'","\\'").replaceAll('"','\\"');
    var   repatomic_name = atomic_name;

    const editorHTML = `
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Monaco Editor</title>
        <style>
            body, html { height: 100%; margin: 0; }
            #container { width: 100%; height: 95%; }
            #button-container { width: 100%; height: 5%; justify-content: flex-end; align-items: left; box-sizing: border-box; padding: 5px;}
        </style>

        <script src="coupling_designer_main2.js"></script>

        <script type="text/javascript">
	window.onload = function() {
		var btn_sim_save = document.getElementById('btn_sim_save');
		btn_sim_save.onclick = function() {
           opener.parent.set_model_on_server("${repatomic_name}", monacoeditor.getValue());
		};
	};
</script>

    </head>
    <body>
        <div id="container">
        <script src="https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.33.0/min/vs/loader.min.js"></script>
        <script>
            require.config({ paths: { 'vs': 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.33.0/min/vs' }});
            require(['vs/editor/editor.main'], function() {
              monacoeditor = monaco.editor.create(document.getElementById('container'), {
                    value: "${repParam}",
	                language: "python",
                    scrollBeyondLastLine: false,
                    automaticLayout: true
                });
            });
        </script>
        </div>
         <div id="button-container">
                    <button class="save" type="button" id="btn_sim_save" style="width:128px;height:32px;float: right;">
                    <img src="images/btn_save.png" style="margin: 0px;" border="0" title="저장">
                    </button>
         </div>
    </body>
    </html>
`
    return editorHTML;
}

function openEditorInNewWindow(atomic_name, param) {

    const editorWindow = window.open('', '', 'width=800,height=800');
    if (editorWindow) {
         editorWindow.document.open();
         editorWindow.document.write(getMonacoDoc(atomic_name, param));
         editorWindow.document.close();
    }
 }

function readCode() {

}