let color_idx_fn = (()=>{
    let rand_color_idx = 0;
    return ()=>{
        rand_color_idx += 77;
        rand_color_idx %= 360;
        console.log('color : ', rand_color_idx);
        return rand_color_idx;
    }
})();

function get_random_color(alpha = 1.0){
    //rand_color_idx += 57;
    //rand_color_idx %= 360;
    return 'hsla(' + color_idx_fn() + ', 100%, 50%, ' + alpha + ')';
    //return 'hsla(' + (Math.random() * 360) + ', 100%, 50%, ' + alpha + ')';
}