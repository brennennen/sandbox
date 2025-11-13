///
/// Example program demostrating a simple website that says hello world.
///

use log;
use env_logger::Env;
use http;

///
/// Callback function made when a request is made to the HTTP server.
///
fn process_request(request: http::Request) -> http::Response {
    log::info!("processing request");
    let mut response = http::Response::new();
    response.status_code = http::StatusCode::Ok;
    let response_text = format!("\
<html>
    <head>
        <style>
            * {{
                font-family: verdana;
            }}
            .container {{
                margin-top: 20px;
                margin-left: 100px;
                margin-right: 100px;
            }}
        </style>
    </head>
    <body>
        <div class='container'>
            <h1>Hello World!</h1>
            <hr />
            <h2>Request</h2>
            <dl>
                <dt>Protocol</dt>   <dd>{}</dd>
                <dt>Method</dt>     <dd>{}</dd>
                <dt>Path</dt>       <dd>{}</dd>
                <dt>Headers</dt>    <dd>{:?}</dd>
            </dl>
            <hr />
            <h2>Response</h2>
            <dl>
                <dt>Status Code</dt>   <dd>{}</dd>
            </dl>
        </div>
    </body>
</html>", request.protocol.to_string(), request.method.to_string(),
request.raw_path, request.headers, response.status_code);
    response.set_content_text(response_text.to_string());
    return response;
}

fn periodic_callback() {
    //log::info!("periodic timeout from in the app!");
}

///
/// Two steps are needed to get the http server running:
///
/// let mut server = http::server::Server::new(port, request_callback);
/// server.start();
///
fn main() {
    env_logger::from_env(Env::default().default_filter_or("debug")).init();
    log::info!("Starting example 'Hello World' server testing the 'http' library.");
    let mut server = http::Server::new(8080, process_request, periodic_callback);
    server.start();
    log::info!("Stopping example 'Hello World' server.");
}
