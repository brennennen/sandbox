///
/// Example program demostrating a simple website that says hello world.
/// 

use std::thread;
use log;
use env_logger::Env;
use http;
use std::net::TcpStream;
use std::io::prelude::*;
use std::time::Instant;

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
            <h2>Request Details</h2>
            <dl>
                <dt>Protocol</dt>   <dd>{}</dd>
                <dt>Method</dt>     <dd>{}</dd>
                <dt>Path</dt>       <dd>{}</dd>
                <dt>Headers</dt>    <dd>{:?}</dd>
            </dl>
            <hr />
            <h2>Response Details</h2>
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

///
/// Two steps are needed to get the http server running:
/// 
/// let mut server = http::server::Server::new(port, request_callback);
/// server.start();
/// 
fn server_main() {
    log::info!("Starting server");
    let mut server = http::Server::new(8080, process_request);
    server.start();
    log::info!("Stopping example 'Hello World' server.");
}

fn tests_main() {
    log::info!("Starting client connection tests...");
    single_connection_test();
    single_connection_test();
    //multiple_connection_test();
}

fn single_connection_test() {
    log::info!("Starting single connection test...");
    let mut client_stream = std::net::TcpStream::connect("127.0.0.1:8080").unwrap();

    let get_request = "GET / HTTP/1.1\r\n\r\n\r\n";
    let mut result_buffer = [0; 16384];

    let start = Instant::now();
    let _ = client_stream.write(&get_request.as_bytes());
    //client_stream.flush();
    let _ = client_stream.read(&mut result_buffer);
    //client_stream.flush();
    let elapsed = start.elapsed();
    log::info!("Elapsed time: {:?}", elapsed);

    //let _ = client_stream.write(&[1]);
    //client_stream.flush();
}

fn multiple_connection_test() {
    log::info!("Starting multiple connection test...");
    //let mut client_stream = std::net::TcpStream::connect("127.0.0.1:8080").unwrap();

    let get_request = "GET / HTTP/1.1\r\n\r\n\r\n";
    let mut result_buffer = [0; 16384];

    let mut index: u32 = 0;

    let multi_start = Instant::now();
    loop {
        let mut client_stream = std::net::TcpStream::connect("127.0.0.1:8080").unwrap();
        index += 1;
        let start = Instant::now();
        let _ = client_stream.write(&get_request.as_bytes());
        let _ = client_stream.read(&mut result_buffer);
        let elapsed = start.elapsed();
        log::info!("Elapsed time: {:?}", elapsed);

        //let _ = client_stream.write(&[1]);
        //client_stream.flush();
        if index >= 1 {
            break;
        }
    }
    let multi_elapsed = multi_start.elapsed();
    log::info!("100 x Elapsed time: {:?}", multi_elapsed);
}

fn main() {
    env_logger::from_env(Env::default().default_filter_or("debug")).init();
    log::info!("Starting benchmarks test");
    let server_thread_handler = std::thread::spawn(|| {
        server_main();
    });

    log::info!("Sleeping for 5 seconds to avoid OS socket startup time skewing results.");
    std::thread::sleep(std::time::Duration::from_millis(1000));

    tests_main();

    server_thread_handler.join().unwrap();
}
