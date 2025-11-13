///
/// HTTP status codes are managed by the IANA:
/// http://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
/// 

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum StatusCode
{
    // Internal (not part of spec)
    Unknown =                       0,

    // Informational 1xx
    Continue =                      100,
    SwitchingProtocols =            101,
    Processing =                    102,
    EarlyHints =                    103,

    // Successful 2xx
    Ok =                            200,
    Created =                       201,
    Accepted =                      202,
    NonAuthoritiveInformation =     203,
    NoContent =                     204, 
    ResetContent =                  205,
    PartialContent =                206,
    MultiStatus =                   207,
    AlreadyReported =               208,
    ImUsed =                        209,

    // Redirection 3xx
    MultipleChoices =               300,
    MovedPermanently =              301,
    Found =                         302,
    SeeOther =                      303,
    NotMethod =                     304,
    UseProxy =                      305,
    Reserved306 =                   306,
    TemporaryRedirect =             307,
    PermanentRedirect =             308,

    // Client Errors 4xx
    BadRequest =                    400,
    Unauthorized =                  401,
    PaymentRequired =               402,
    Forbidden =                     403,
    NotFound =                      404,
    MethodNotAllowed =              405,
    NotAcceptable =                 406,
    ProxyAuthenticationRequired =   407,
    RequestTimeout =                408,
    Conflict =                      409,
    Gone =                          410,
    LengthRequired =                411,
    PreconditionFailed =            412,
    RequestEntityTooLarge =         413,
    RequestUriTooLong =             414,
    UnsupportedMediaType =          415,
    RequestedRangeNotSatisfiable =  416,
    ExpectationFailed =             417,
    ImATeapot =                     418,
    MisdirectedRequest =            421,
    UnprocessableEntity =           422,
    Locked =                        423,
    FailedDependency =              424,
    TooEarly =                      425,
    UpgradeRequired =               426,
    PreconditionRequired =          428,
    TooManyRequests =               429,
    RequestHeaderFieldsTooLarge =   430,
    UnavailableForLegalReasons =    451,

    // Server Error 5xx
    InternalServerError =           500,
    NotImplemented =                501,
    BadGateway =                    502,
    ServiceUnavailable =            503,
    GatewayTimeout =                504,
    HttpVersionNotSupported =       505,
    VariantAlsoNegotiates =         506,
    InsufficientStorage =           507,
    LoopDetected =                  508,
    NotExtended =                   510,
    NetworkAuthenticationRequired = 511
}

///
/// Convert an HTTP Status Code from a string to an enumeration.
/// 
/// ```
/// let status: http::status_code::StatusCode = "Internal Server Error".parse().unwrap();
/// assert_eq!(status, http::status_code::StatusCode::InternalServerError);
/// ```
impl std::str::FromStr for StatusCode {
    type Err = String;

    fn from_str(string: &str) -> Result<Self, Self::Err> {
        match string {
            // Internal (not part of spec)
            "Unknown" =>                        Ok(StatusCode::Unknown),

            // Informational 1xx
            "Continue" =>                       Ok(StatusCode::Continue),
            "Switching Protocols" =>            Ok(StatusCode::SwitchingProtocols),
            "Processing" =>                     Ok(StatusCode::Processing),
            "Early Hints" =>                    Ok(StatusCode::EarlyHints),

            // Successful 2xx
            "OK" =>                             Ok(StatusCode::Ok),
            "Created" =>                        Ok(StatusCode::Created),
            "Accepted" =>                       Ok(StatusCode::Accepted),
            "Non-Authoritive Information" =>    Ok(StatusCode::NonAuthoritiveInformation),
            "No Content" =>                     Ok(StatusCode::NoContent),
            "Reset Content" =>                  Ok(StatusCode::ResetContent),
            "Partial Content" =>                Ok(StatusCode::PartialContent),
            "Multi Status" =>                   Ok(StatusCode::MultiStatus),
            "Already Reported" =>               Ok(StatusCode::AlreadyReported),
            "Im Used" =>                        Ok(StatusCode::ImUsed),

            // Redirection 3xx
            "Multiple Choices" =>               Ok(StatusCode::MultipleChoices),
            "Moved Permanently" =>              Ok(StatusCode::MovedPermanently),
            "Found" =>                          Ok(StatusCode::Found),
            "See Other" =>                      Ok(StatusCode::SeeOther),
            "Not Method" =>                     Ok(StatusCode::NotMethod),
            "Use Proxy" =>                      Ok(StatusCode::UseProxy),
            "Reserved306" =>                    Ok(StatusCode::Reserved306),
            "Temporary Redirect" =>             Ok(StatusCode::TemporaryRedirect),
            "Permanent Redirect" =>             Ok(StatusCode::PermanentRedirect),

            // Client Errors 4xx
            "Bad Request" =>                    Ok(StatusCode::BadRequest),
            "Unauthorized" =>                   Ok(StatusCode::Unauthorized),
            "Payment Required" =>               Ok(StatusCode::PaymentRequired),
            "Forbidden" =>                      Ok(StatusCode::Forbidden),
            "Not Found" =>                      Ok(StatusCode::NotFound),
            "Method Not Allowed" =>             Ok(StatusCode::MethodNotAllowed),
            "Not Acceptable" =>                 Ok(StatusCode::NotAcceptable),
            "Proxy Authentication Required" =>  Ok(StatusCode::ProxyAuthenticationRequired),
            "Request Timeout" =>                Ok(StatusCode::RequestTimeout),
            "Conflict" =>                       Ok(StatusCode::Conflict),
            "Gone" =>                           Ok(StatusCode::Gone),
            "Length Required" =>                Ok(StatusCode::LengthRequired),
            "Precondition Failed" =>            Ok(StatusCode::PreconditionFailed),
            "Request Entity Too Large" =>       Ok(StatusCode::RequestEntityTooLarge),
            "Request Uri Too Long" =>           Ok(StatusCode::RequestUriTooLong),
            "Unsupported Media Type" =>         Ok(StatusCode::UnsupportedMediaType),
            "Requested Range Not Satisfiable" => Ok(StatusCode::RequestedRangeNotSatisfiable),
            "Expectation Failed" =>             Ok(StatusCode::ExpectationFailed),
            "Im A Teapot" =>                    Ok(StatusCode::ImATeapot),
            "Misdirected Request" =>            Ok(StatusCode::MisdirectedRequest),
            "Unprocessable Entity" =>           Ok(StatusCode::UnprocessableEntity),
            "Locked" =>                         Ok(StatusCode::Locked),
            "Failed Dependency" =>              Ok(StatusCode::FailedDependency),
            "Too Early" =>                      Ok(StatusCode::TooEarly),
            "Upgrade Required" =>               Ok(StatusCode::UpgradeRequired),
            "Precondition Required" =>          Ok(StatusCode::PreconditionRequired),
            "Too Many Requests" =>              Ok(StatusCode::TooManyRequests),
            "Request Header Fields Too Large" => Ok(StatusCode::RequestHeaderFieldsTooLarge),
            "Unavailable For Legal Reasons" =>  Ok(StatusCode::UnavailableForLegalReasons),

            // Server Error 5xx
            "Internal Server Error" =>          Ok(StatusCode::InternalServerError),
            "Not Implemented" =>                Ok(StatusCode::NotImplemented),
            "Bad Gateway" =>                    Ok(StatusCode::BadGateway),
            "Service Unavailable" =>            Ok(StatusCode::ServiceUnavailable),
            "Gateway Timeout" =>                Ok(StatusCode::GatewayTimeout),
            "Http Version Not Supported" =>     Ok(StatusCode::HttpVersionNotSupported),
            "Variant Also Negotiates" =>        Ok(StatusCode::VariantAlsoNegotiates),
            "Insufficient Storage" =>           Ok(StatusCode::InsufficientStorage),
            "Loop Detected" =>                  Ok(StatusCode::LoopDetected),
            "Not Extended" =>                   Ok(StatusCode::NotExtended),
            "Network Authentication Required" => Ok(StatusCode::NetworkAuthenticationRequired),

            _ => Err(format!("'{}' is not a valid value for Http::StatusCode", string))
        }
    }
}

///
/// Convert an HTTP Status Code from a string to an enumeration.
/// 
/// ```
/// let status: http::status_code::StatusCode = "Internal Server Error".parse().unwrap();
/// assert_eq!(status, http::status_code::StatusCode::InternalServerError);
/// ```
impl std::fmt::Display for StatusCode {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match *self {
            // Internal (not part of spec)
            StatusCode::Unknown                 => write!(f, "Unknown"),

            // Informational 1xx
            StatusCode::Continue                => write!(f, "Continue"),
            StatusCode::SwitchingProtocols      => write!(f, "Switching Protocols"),
            StatusCode::Processing              => write!(f, "Processing"),
            StatusCode::EarlyHints              => write!(f, "Early Hints"),

            // Successful 2xx
            StatusCode::Ok                      => write!(f, "OK"),
            StatusCode::Created                 => write!(f, "Created"),
            StatusCode::Accepted                => write!(f, "Accepted"),
            StatusCode::NonAuthoritiveInformation => write!(f, "Non-Authoritive Information"),
            StatusCode::NoContent               => write!(f, "No Content"),
            StatusCode::ResetContent            => write!(f, "Reset Content"),
            StatusCode::PartialContent          => write!(f, "Partial Content"),
            StatusCode::MultiStatus             => write!(f, "Multi Status"),
            StatusCode::AlreadyReported         => write!(f, "Already Reported"),
            StatusCode::ImUsed                  => write!(f, "IM Used"),

            // Redirection 3xx
            StatusCode::MultipleChoices         => write!(f, "Multiple Choices"),
            StatusCode::MovedPermanently        => write!(f, "Moved Permanently"),
            StatusCode::Found                   => write!(f, "Found"),
            StatusCode::SeeOther                => write!(f, "See Other"),
            StatusCode::NotMethod               => write!(f, "Not Method"),
            StatusCode::UseProxy                => write!(f, "Use Proxy"),
            StatusCode::Reserved306             => write!(f, "Reserved306"),
            StatusCode::TemporaryRedirect       => write!(f, "Temporary Redirect"),
            StatusCode::PermanentRedirect       => write!(f, "Permanent Redirect"),

            // Client Errors 4xx
            StatusCode::BadRequest              => write!(f, "Bad Request"),
            StatusCode::Unauthorized            => write!(f, "Unauthorized"),
            StatusCode::PaymentRequired         => write!(f, "Payment Required"),
            StatusCode::Forbidden               => write!(f, "Forbidden"),
            StatusCode::NotFound                => write!(f, "Not Found"),
            StatusCode::MethodNotAllowed        => write!(f, "Method Not Allowed"),
            StatusCode::NotAcceptable           => write!(f, "Not Acceptable"),
            StatusCode::ProxyAuthenticationRequired => write!(f, "Proxy Authentication Required"),
            StatusCode::RequestTimeout          => write!(f, "Request Timeout"),
            StatusCode::Conflict                => write!(f, "Conflict"),
            StatusCode::Gone                    => write!(f, "Gone"),
            StatusCode::LengthRequired          => write!(f, "Length Required"),
            StatusCode::PreconditionFailed      => write!(f, "Precondition Failed"),
            StatusCode::RequestEntityTooLarge   => write!(f, "Request Entity Too Large"),
            StatusCode::RequestUriTooLong       => write!(f, "Request URI Too Long"),
            StatusCode::UnsupportedMediaType    => write!(f, "Unsupported Media Type"),
            StatusCode::RequestedRangeNotSatisfiable => write!(f, "Requested Range Not Satisfiable"),
            StatusCode::ExpectationFailed       => write!(f, "Expectation Failed"),
            StatusCode::ImATeapot               => write!(f, "I'm A Teapot"),
            StatusCode::MisdirectedRequest      => write!(f, "Misdirected Request"),
            StatusCode::UnprocessableEntity     => write!(f, "Unprocessable Entity"),
            StatusCode::Locked                  => write!(f, "Locked"),
            StatusCode::FailedDependency        => write!(f, "Failed Dependency"),
            StatusCode::TooEarly                => write!(f, "Too Early"),
            StatusCode::UpgradeRequired         => write!(f, "Upgrade Required"),
            StatusCode::PreconditionRequired    => write!(f, "Precondition Required"),
            StatusCode::TooManyRequests         => write!(f, "Too Many Requests"),
            StatusCode::RequestHeaderFieldsTooLarge => write!(f, "Request Header Fields Too Large"),
            StatusCode::UnavailableForLegalReasons => write!(f, "Unavailable For Legal Reasons"),
            
            // Server Error 5xx
            StatusCode::InternalServerError     => write!(f, "Internal Server Error"),
            StatusCode::NotImplemented          => write!(f, "Not Implemented"),
            StatusCode::BadGateway              => write!(f, "Bad Gateway"),
            StatusCode::ServiceUnavailable      => write!(f, "Service Unavailable"),
            StatusCode::GatewayTimeout          => write!(f, "Gateway Timeout"),
            StatusCode::HttpVersionNotSupported => write!(f, "HTTP Version Not Supported"),
            StatusCode::VariantAlsoNegotiates   => write!(f, "Variant Also Negotiates"),
            StatusCode::InsufficientStorage     => write!(f, "Insufficient Storage"),
            StatusCode::LoopDetected            => write!(f, "Loop Detected"),
            StatusCode::NotExtended             => write!(f, "Not Extended"),
            StatusCode::NetworkAuthenticationRequired => write!(f, "Network Authentication Required"),
        }
    }
}
