

#[derive(Debug)]
pub struct ServerMetrics
{
    pub raw_request_count: u32,
    pub page_request_count: u32
}

impl ServerMetrics {
    pub fn new() -> ServerMetrics {
        let mut server_metrics = ServerMetrics {
            raw_request_count: 0,
            page_request_count: 0
        };
        return server_metrics;
    }

    // pub fn to_string() {

    // }
}


