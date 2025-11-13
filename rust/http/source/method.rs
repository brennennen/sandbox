///
/// HTTP methods were defined in the original HTTP/1.0 specification
/// and have been added to over the years.
/// * RFC 1945 (HTTP/1.0)
///     * GET
///     * HEAD
///     * POST
/// * RFC 2616 (HTTP/1.1)
/// * RFC 7231 (HTTP/1.1)
///     * PUT
///     * DELETE
///     * CONNECT
///     * OPTIONS
///     * TRACE
/// * RFC 5789 (PATCH method)
///     * PATCH
/// * RFC 7540 (HTTP/2.0)
///     * CONNECT (Redefined)
/// 

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum Method
{
    Unknown,
    Get,
    Head,
    Post,
    Put,
    Delete,
    Connect,
    Trace,
    Patch
}

///
/// Convert an HTTP Method from a string to an enumeration.
/// 
/// ```
/// let method: http::method::Method = "POST".parse().unwrap();
/// assert_eq!(method, http::method::Method::Post);
/// ```
impl std::str::FromStr for Method {
    type Err = String;

    fn from_str(string: &str) -> Result<Self, Self::Err> {
        match string {
            "GET" => Ok(Method::Get),
            "HEAD" => Ok(Method::Head),
            "POST" => Ok(Method::Post),
            "PUT" => Ok(Method::Put),
            "DELETE" => Ok(Method::Delete),
            "CONNECT" => Ok(Method::Connect),
            "TRACE" => Ok(Method::Trace),
            "PATCH" => Ok(Method::Patch),
            _ => Err(format!("'{}' is not a valid value for Http::Method", string))
        }
    }
}

///
/// Convert an HTTP Method from an enumeration to a string.
/// 
/// ```
/// let method_string = format!("{}", http::method::Method::Trace); 
/// assert_eq!(method_string, "TRACE");
/// ```
impl std::fmt::Display for Method {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match *self {
            Method::Unknown => write!(f, "UNKNOWN"),
            Method::Get     => write!(f, "GET"),
            Method::Head    => write!(f, "HEAD"),
            Method::Post    => write!(f, "POST"),
            Method::Put     => write!(f, "PUT"),
            Method::Delete  => write!(f, "DELETE"),
            Method::Connect => write!(f, "CONNECT"),
            Method::Trace   => write!(f, "TRACE"),
            Method::Patch   => write!(f, "PATCH")
        }
    }
}
