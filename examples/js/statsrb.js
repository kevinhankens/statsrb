/**
 * @file
 * This javascript obtains metrics from Statsrb and displays them using a simple
 * Flot time series graph.
 */

$(document).ready(function (context) {

  // Your data namespace to query, statsrb rack server domain name and port.
  var namespace = 'test';
  var domain = 'localhost';
  var port = '9292';

  // Plot options.
  var options = {
    xaxis: {
      mode: "time",
      timeformat: "%m/%d %I:%M"
    },
  }

  // Get the data from the server and plot it.
  $.ajax({type: 'GET',
    url: 'http://' + domain + ':' + port + '/get/' + namespace,
    crossDomain: true,
    dataType: 'jsonp',
    jsonp: 'jsoncallback',
    success: function(data) {
      $.each(data[namespace], function(key, value) {
        data[namespace][key][0] *= 1000;
      });

      $('#stats').before('<h1>Stats for: &quot;' + namespace + '&quot;</h1>');
      $.plot('#stats', [data[namespace]], options);
    }
  });

});
