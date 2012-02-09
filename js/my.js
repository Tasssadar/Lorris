function do_scroll(where)
{
    if(where == '')
        return;
    
    $('html, body').animate({
        scrollTop: $(where).offset().top-45
    }, 1000);

   // location.hash = where;
}