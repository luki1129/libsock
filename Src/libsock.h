#pragma once
#ifndef __libsock_h_
#define __libsock_h_
#ifndef RC_INVOKED
#include <utility>
#include <memory>
#include <exception>
#include <system_error>
#include <string>
#include <sstream>
#include <vector>
#include <type_traits>
#include <algorithm>

#ifndef _CONSTEXPR_IF
#if defined( __cpp_if_constexpr )
#if __cpp_if_constexpr <= __cplusplus
#define _CONSTEXPR_IF constexpr
#else
#define _CONSTEXPR_IF
#endif
#else
#define _CONSTEXPR_IF
#endif
#endif

#ifndef _NODISCARD
#if defined( __has_cpp_attribute )
#if __has_cpp_attribute( nodiscard )
#define _NODISCARD [[nodiscard]]
#else
#define _NODISCARD
#endif
#else
#define _NODISCARD
#endif
#endif

#if defined( _WIN32 ) || defined( _WIN64 ) || defined( WIN32 )
#define OS_WINDOWS
#define NOMINMAX
#include <sdkddkver.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#undef NOMINMAX

#define strcpy strcpy_s
#define wcscpy wcscpy_s

#elif defined( __linux__ )
#define OS_LINUX
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#else
#error Unknown target OS

#endif

#define _LIBSOCK ::libsock::

#define _LIBSOCK_CHECK_ARG_NOT_NULL( ARG ) \
    if( (ARG) == nullptr ) \
        throw std::invalid_argument( #ARG " cannot be NULL" );

#define _LIBSOCK_CHECK_ARG_NOT_EQ( ARG, VAL ) \
    if( (ARG) == (VAL) ) \
        throw std::invalid_argument( #ARG " cannot be " #VAL ); 


namespace libsock
{

template<typename _Ty, typename _Ty2>
inline _Ty _Reinterpret_optional_or_default( const _Ty2* _Optional, _Ty _Default )
    {   // extract value from optional value via reinterpret_cast
    return ((_Optional != nullptr) ? *reinterpret_cast<const _Ty*>(_Optional) : _Default);
    }

template<typename _Ty, typename _Ty2>
inline _Ty _Static_optional_or_default( const _Ty2* _Optional, _Ty _Default )
    {   // extract value from optional value via static_cast
    return ((_Optional != nullptr) ? static_cast<_Ty>(*_Optional) : _Default);
    }

template<typename _Ty>
inline void _Array_delete( _Ty _Array[] ) noexcept
    {   // free array of elements
    if( _Array != nullptr )
        delete[] _Array;
    }

template<typename _Elem, typename _Ty>
inline std::basic_string<_Elem> _To_string( _Ty _Val,
        typename std::enable_if<std::is_arithmetic<_Ty>::value>::type* = nullptr )
    {
    return std::to_string( _Val );
    }

template<typename _Ty>
inline std::basic_string<wchar_t> _To_string( _Ty _Val,
        typename std::enable_if<std::is_arithmetic<_Ty>::value>::type* = nullptr )
    {
    return std::to_wstring( _Val );
    }


// CLASS TEMPLATE _Socket_flags_helper
template<typename _FlagTy, typename _StorageTy>
class _Socket_flags_helper
    {
public:
    typedef _StorageTy storage_type;
    typedef _FlagTy flag_type;

    inline _Socket_flags_helper() noexcept
        : _MyValue( 0 )
        {   // construct empty flags helper
        }

    inline _Socket_flags_helper( flag_type _Flag ) noexcept
        : _MyValue( static_cast<storage_type>(_Flag) )
        {   // construct flags helper from single flag
        }

    inline explicit _Socket_flags_helper( storage_type _Value ) noexcept
        : _MyValue( _Value )
        {   // construct from storage value
        }

    _NODISCARD inline _Socket_flags_helper operator|( const _Socket_flags_helper& _Right ) const noexcept
        {   // set flags
        return _Socket_flags_helper( this->_MyValue | _Right._MyValue );
        }

    inline _Socket_flags_helper& operator|=( const _Socket_flags_helper& _Right ) noexcept
        {   // set flags
        this->_MyValue |= _Right._MyValue;
        return (*this);
        }

    _NODISCARD inline _Socket_flags_helper operator&( const _Socket_flags_helper& _Right ) const noexcept
        {   // filter flags
        return _Socket_flags_helper( this->_MyValue & _Right._MyValue );
        }

    inline _Socket_flags_helper& operator&=( const _Socket_flags_helper& _Right ) noexcept
        {   // filter flags
        this->_MyValue &= _Right._MyValue;
        return (*this);
        }

    _NODISCARD inline bool operator==( const _Socket_flags_helper& _Right ) const noexcept
        {   // compare flags
        return this->_MyValue == _Right._MyValue;
        }

    _NODISCARD inline bool operator!=( const _Socket_flags_helper& _Right ) const noexcept
        {   // compare flags
        return !this->operator==( _Right );
        }

    _NODISCARD inline explicit operator const storage_type() const noexcept
        {   // cast flags into storage type
        return this->_MyValue;
        }

protected:
    storage_type _MyValue;
    };


template<typename _FlagTy, typename _StorageTy>
_NODISCARD _Socket_flags_helper<_FlagTy, _StorageTy> operator|( _FlagTy _Flag, const _Socket_flags_helper<_FlagTy, _StorageTy>& _Flags ) noexcept
    {   // combine single flag with multiple flags under _Socket_flags_helper
    return _Flags | _Flag;
    }

template<typename _FlagTy, typename _StorageTy>
_NODISCARD _Socket_flags_helper<_FlagTy, _StorageTy> operator&( _FlagTy _Flag, const _Socket_flags_helper<_FlagTy, _StorageTy>& _Flags ) noexcept
    {   // filter single flag with multiple flags under _Socket_flags_helper
    return _Flags & _Flag;
    }


#if defined( OS_WINDOWS )
typedef SOCKET _Socket_handle;
typedef int _Sock_size_t;
typedef int _Sockcomm_data_size_t;
typedef char _Sockcomm_data_t;
typedef char _Sockopt_data_t;
constexpr _Socket_handle _Invalid_socket = INVALID_SOCKET;

typedef SOCKADDR_IN _Sockaddr_inet;
typedef SOCKADDR_IN6 _Sockaddr_inet6;

#elif defined( OS_LINUX )
typedef int _Socket_handle;
typedef socklen_t _Sock_size_t;
typedef size_t _Sockcomm_data_size_t;
typedef void _Sockcomm_data_t;
typedef void _Sockopt_data_t;
constexpr _Socket_handle _Invalid_socket = -1;

typedef sockaddr_in _Sockaddr_inet;
typedef sockaddr_in6 _Sockaddr_inet6;

#else
#error socket types not defined for this OS
#endif

namespace __impl
{
#if defined( OS_WINDOWS ) \
 || defined( OS_LINUX )
using ::socket;
using ::shutdown;
using ::connect;
using ::accept;
using ::bind;
using ::listen;
using ::setsockopt;
using ::getsockopt;
using ::send;
using ::sendto;
using ::recv;
using ::recvfrom;
using ::getprotobyname;
using ::getaddrinfo;
using ::freeaddrinfo;

using ::memcpy;
using ::memset;
using ::strlen;
using ::strcpy;
using ::wcslen;
using ::wcscpy;

using ::std::swap;
using ::std::move;
using ::std::min;
using ::std::max;

#if defined( OS_WINDOWS )
using ::closesocket;
#elif defined( OS_LINUX )
inline int closesocket( _Socket_handle _Socket ) noexcept
    {   // closesocket alias for linux
    return ::close( _Socket );
    }
#endif

#else
#error Socket functions not defined for this OS
#endif

inline int geterror( int _Retval ) noexcept
    {   // gets last error reported by the sockets API
    (_Retval);
#if defined( OS_WINDOWS )
    return WSAGetLastError();
#elif defined( OS_LINUX )
    return errno;
#else
    return _Retval;
#endif
    }
}


// CLASS _Socket_error_category
class _Socket_error_category
    : public std::error_category
    {
public:
    _NODISCARD inline virtual const char* name() const noexcept override
        {
        return "libsock error";
        }

    _NODISCARD inline virtual std::string message( int _Errval ) const override
        {
        std::string msg = "Unable to retrieve error message";
#   if defined( OS_WINDOWS )
        // Windows OSes provide FormatMessage function, which translates error messages
        // into human-readable forms.
        char* msg_buffer = nullptr;
        ::FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            _Errval,
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
            (char*)(&msg_buffer), 0,
            nullptr );
        if( msg_buffer != nullptr )
            { // Copy obtained message to the output string and free the memory.
            msg.assign( msg_buffer );
            ::LocalFree( msg_buffer );
            }
#   elif defined( OS_LINUX )
        // Linux OSes provide strerror function, which translatess errno messages into
        // human-readable forms.
        msg.assign( ::strerror( _Errval ) );
#   else
#   error _Socket_error_category::message: Messages not implemented for this OS
#   endif
        return msg;
        }
    };


// CLASS socket_exception
class socket_exception
    : public std::system_error
    {
    typedef std::system_error _MyBase;

public:
    inline socket_exception( int _Errval )
        : _MyBase( _Errval, _Socket_error_category{} )
        {   // construct basic socket exception
        }

    inline socket_exception( int _Errval, const char* _Message )
        : _MyBase( _Errval, _Socket_error_category{}, _Message )
        {   // construct basic socket exception with own message
        }
    };


// STRUCT TEMPLATE big_endian
template<typename _Ty>
struct big_endian
    {
    _Ty raw;

    inline big_endian<_Ty>()
        : raw( 0 )
        {   // construct uninitialized big_endian value
        }

    inline big_endian<_Ty>( _Ty _Val, bool _Raw = false )
        : raw( 0 )
        {   // construct big_endian value from host value
        if( _Raw )
            this->raw = _Val;
        else
            _Assign_from_host( _Val );
        }

    inline operator _Ty() const
        {   // convert big_endian value to host value
        return this->raw;
        }

    inline _Ty as_big_endian() const
        {   // return big-endian representation
        return this->raw;
        }

    inline _Ty as_little_endian() const
        {   // return little-endian representation
        return _Bswap( this->raw );
        }

    inline _Ty as_host_endian() const
        {   // return host-endian representation
        if( _Is_little_endian_host() )
            return as_little_endian();
        else
            return as_big_endian();
        }

protected:
    inline constexpr bool _Is_little_endian_host() const
        {   // check if host machine is little-endian
        const long _Num = 0x12345678;
        return (int)(*(const char*)&_Num) == 0x78;
        }

    inline void _Assign_from_host( _Ty _Val )
        {   // assign value from host-endian representation
        if( _Is_little_endian_host() )
            this->raw = _Bswap( _Val );
        else
            this->raw = _Val;
        }

    inline _Ty _Bswap( _Ty _Val ) const
        {   // swap byte order of the value
        if _CONSTEXPR_IF( sizeof( _Ty ) == sizeof( unsigned short ) )
            { // 2-byte values
            auto _Bytes = _Cvt<unsigned short>( _Val );
            _Bytes = 
                ((_Bytes << 8) & 0xFF00) |
                ((_Bytes >> 8) & 0x00FF);
            return _Cvt<_Ty>( _Bytes );
            }
        else if _CONSTEXPR_IF( sizeof( _Ty ) == 4 )
            { // 4-byte values
            auto _Bytes = _Cvt<unsigned long>( _Val );
            _Bytes = 
                ((_Bytes << 24) & 0xFF000000) |
                ((_Bytes << 8 ) & 0x00FF0000) |
                ((_Bytes >> 8 ) & 0x0000FF00) |
                ((_Bytes >> 24) & 0x000000FF);
            return _Cvt<_Ty>( _Bytes );
            }
        else if _CONSTEXPR_IF( sizeof( _Ty ) == 8 )
            { // 8-byte values
            auto _Bytes = _Cvt<unsigned long long>( _Val );
            _Bytes =
                ((_Bytes << 56) & 0xFF00000000000000) |
                ((_Bytes << 40) & 0x00FF000000000000) |
                ((_Bytes << 24) & 0x0000FF0000000000) |
                ((_Bytes << 8 ) & 0x000000FF00000000) |
                ((_Bytes >> 8 ) & 0x00000000FF000000) |
                ((_Bytes >> 24) & 0x0000000000FF0000) |
                ((_Bytes >> 40) & 0x000000000000FF00) |
                ((_Bytes >> 56) & 0x00000000000000FF);
            return _Cvt<_Ty>( _Bytes );
            }
        else if _CONSTEXPR_IF( sizeof( _Ty ) > 8 )
            { // other values
            throw std::runtime_error(
                __FUNCTION__ " is not implemented for this argument size" );
            }
        else
            { // 1-byte values
            return _Val;
            }
        }

    template<typename _Tx, typename _Ux>
    inline _Tx _Cvt( _Ux _Val ) const
        {   // reinterpret value bytes
        return *reinterpret_cast<_Tx*>(&_Val);
        }
    };

template<typename _Ty>
inline big_endian<_Ty> make_big_endian( _Ty _Val, bool _Raw = false )
    {   // construct big_endian value
    return big_endian<_Ty>( _Val, _Raw );
    }


// CLASS libsock_scope
class libsock_scope
    {
public:
    inline libsock_scope()
        {   // initialize socket library
#   if defined( OS_WINDOWS )
        WSADATA wsaData;
        if( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) != ERROR_SUCCESS )
            throw socket_exception( ::WSAGetLastError() );
        (wsaData); // avoid 'not referenced' warnings
#   endif
        }

    inline ~libsock_scope() noexcept
        {   // deinitialize socket library
#   if defined( OS_WINDOWS )
        ::WSACleanup();
#   endif
        }
    };


// ENUM CLASS address_family
enum class socket_address_family
    {
    unknown             = -1,               // Unknown
    unspec              = AF_UNSPEC,        // Unspecified
    local               = AF_UNIX,          // Local to host (pipes, portals)
    inet                = AF_INET,          // Internet IP protocol version 4 (IPv4)
    inet6               = AF_INET6,         // Internet IP protocol version 6 (IPv6)
    irda                = AF_IRDA,          // IrDA
#if defined( OS_WINDOWS )
    atm                 = AF_ATM,           // Native ATM services
    bluetooth           = AF_BTH,           // Bluetooth RFCOMM/L2CAP protocols
#elif defined( OS_LINUX )
    atm                 = AF_ATMSVC,        // Native ATM services
    bluetooth           = AF_BLUETOOTH,     // Bluetooth RFCOMM/L2CAP protocols
#endif
    };


// ENUM CLASS socket_type
enum class socket_type
    {
    unknown             = -1,               //
    stream              = SOCK_STREAM,      // Reliable stream socket
    datagram            = SOCK_DGRAM,       // Unreliable datagram socket
    rdm                 = SOCK_RDM,         // Reliable datagram socket
    seqpacket           = SOCK_SEQPACKET,   // Pseudo-stream datagram socket
    raw                 = SOCK_RAW          // Raw socket
    };


// RAW socket protocol initializer
enum _Raw_proto { _Raw };


// CLASS protocol
class socket_protocol
    {
public:
    inline socket_protocol()
        : _MyId( -1 )
        {   // construct uninitialized (unknown) protocol wrapper
        }

    inline socket_protocol( const char* _Name )
        : _MyId( -1 )
        {   // construct protocol wrapper from IANA name
        _LIBSOCK_CHECK_ARG_NOT_NULL( _Name );
        protoent _Proto_ent = _Get_protocol_from_name( _Name );
        _MyId = static_cast<int>(_Proto_ent.p_proto);
        }

    inline socket_protocol( int _Id ) noexcept
        : _MyId( _Id )
        {   // construct protocol wrapper from IANA id
        }

    inline socket_protocol( _Raw_proto ) noexcept
        : _MyId( 0 )
        {   // construct raw (no) protocol wrapper
        }

    _NODISCARD inline int get_id() const
        {   // get protocol id
        return _MyId;
        }

    _NODISCARD inline explicit operator int() const
        {   // cast protocol into INT value
        return get_id();
        }

protected:
    int _MyId;

    _NODISCARD inline protoent _Get_protocol_from_name( const char* _Name ) const
        {   // get protoent structure from IANA name
        protoent* _Proto_ent = __impl::getprotobyname( _Name );
        if( _Proto_ent == nullptr )
            { // protocol not found
            throw socket_exception( -1, "Unsupported protocol" );
            }
        return (*_Proto_ent);
        }
    };


_NODISCARD inline socket_protocol unknown_socket_protocol() { return socket_protocol(); }
_NODISCARD inline socket_protocol raw_socket_protocol() { return socket_protocol( _Raw ); }
_NODISCARD inline socket_protocol icmp_socket_protocol() { return socket_protocol( "icmp" ); }
_NODISCARD inline socket_protocol igmp_socket_protocol() { return socket_protocol( "igmp" ); }
_NODISCARD inline socket_protocol tcp_socket_protocol() { return socket_protocol( "tcp" ); }
_NODISCARD inline socket_protocol udp_socket_protocol() { return socket_protocol( "udp" ); }


template<typename _SockOptT>
struct _Socket_opt_level;


// ENUM CLASS socket_opt
enum class socket_opt
    {
    unknown = -1,
    reuse_addr          = SO_REUSEADDR,     // allow local address reuse
    keep_alive          = SO_KEEPALIVE,     // keep connections alive
    broadcast           = SO_BROADCAST,     // permit sending of broadcast messages
    //loopback            = SO_USELOOPBACK,   // bypass hardware when possible
    };

template<>
struct _Socket_opt_level<socket_opt>
    {
    static constexpr int value = SOL_SOCKET;
    };


// ENUM CLASS socket_opt_ip
enum class socket_opt_ip
    {
    unknown             = -1,
    join_group          = IP_ADD_MEMBERSHIP,
    leave_group         = IP_DROP_MEMBERSHIP,
    join_source_group   = IP_ADD_SOURCE_MEMBERSHIP,
    leave_source_group  = IP_DROP_SOURCE_MEMBERSHIP,
    block_source        = IP_BLOCK_SOURCE,
    unblock_source      = IP_UNBLOCK_SOURCE,
    header_included     = IP_HDRINCL,
    multicast_interface = IP_MULTICAST_IF,
    multicast_loop      = IP_MULTICAST_LOOP,
    multicast_ttl       = IP_MULTICAST_TTL,
    ttl                 = IP_TTL,
    type_of_service     = IP_TOS,
    unicast_interface   = IP_UNICAST_IF,
    packet_info         = IP_PKTINFO,
#if defined( OS_WINDOWS )
    dont_fragment       = IP_DONTFRAGMENT,
#if defined IP_MTU
    mtu                 = IP_MTU,
    mtu_discover        = IP_MTU_DISCOVER,
#endif
#elif defined( OS_LINUX )
    dont_fragment       = 0x7f000001, // use special value to fallback to MTU_DISCOVER
    mtu                 = IP_MTU,
    mtu_discover        = IP_MTU_DISCOVER,
#else
#error dont_fragment not defined
#endif
    };

template<>
struct _Socket_opt_level<socket_opt_ip>
    { 
    static constexpr int value = IPPROTO_IP;
    static constexpr int value_v6 = IPPROTO_IPV6;
    };


// STRUCT _Socket_address_base
struct _Socket_address_base
    {
public:
    inline _Socket_address_base( socket_address_family _Family ) noexcept
        : _MyFamily( _Family )
        {   // construct socket address base instance
        }

    inline virtual ~_Socket_address_base() noexcept
        {   // destroy socket address base instance
        }
    
public:
    _NODISCARD inline socket_address_family get_family() const noexcept
        {   // get socket address family
        return this->_MyFamily;
        }

    _NODISCARD virtual const sockaddr* get_native_sockaddr() const noexcept = 0;
    _NODISCARD virtual const size_t get_native_sockaddr_size() const noexcept = 0;

protected:
    socket_address_family _MyFamily;
    };


// STRUCT TEMPLATE socket_address
template<socket_address_family _Family, typename _SockAddrTy>
struct socket_address
    : public _Socket_address_base
    {
public:
    inline socket_address() noexcept
        : _Socket_address_base( _Family )
        {   // construct uninitialized socket address wrapper
        }

    inline socket_address( const _SockAddrTy* _Sockaddr ) noexcept
        : _Socket_address_base( _Family ), _MySockaddr()
        {   // construct socket address wrapper
        if( _Sockaddr != nullptr )
            _MySockaddr = (*_Sockaddr);
        }

    _NODISCARD inline virtual const sockaddr* get_native_sockaddr() const noexcept override
        {   // get native sockaddr structure
        return reinterpret_cast<const sockaddr*>(&_MySockaddr);
        }

    _NODISCARD inline virtual const size_t get_native_sockaddr_size() const noexcept override
        {   // get native sockaddr structure size
        return sizeof( _MySockaddr );
        }

protected:
    mutable _SockAddrTy _MySockaddr;
    };


using socket_address_inet = socket_address<socket_address_family::inet, _Sockaddr_inet>;
using socket_address_inet6 = socket_address<socket_address_family::inet6, _Sockaddr_inet6>;


_NODISCARD inline std::shared_ptr<_Socket_address_base> _Create_socket_address( socket_address_family _Family, const sockaddr* _Sockaddr )
    {   // construct socket_address structure based on the family
    // Helper macro for socket_address structure creation
#   define _CASE_ADDRESS_FAMILY(AF) \
    case socket_address_family::AF: \
        { \
        return std::make_shared<socket_address_##AF>( reinterpret_cast<const _Sockaddr_##AF*>(_Sockaddr) ); \
        }
    switch( _Family )
        {
        _CASE_ADDRESS_FAMILY( inet );
        _CASE_ADDRESS_FAMILY( inet6 );
        }
#   undef _CASE_ADDRESS_FAMILY
    throw socket_exception( -1, "address family not supported" );
    }


// ENUM CLASS socket_address_flags
enum class socket_address_flags
    {
    none                = 0,
    passive             = AI_PASSIVE,
    canonname           = AI_CANONNAME,
    };

using _Socket_address_flags_helper = _Socket_flags_helper<socket_address_flags, int>;

_NODISCARD _Socket_address_flags_helper operator|( socket_address_flags _1, socket_address_flags _2 ) noexcept
    {   // construct socket address flags helper from two flags
    return _Socket_address_flags_helper( _1 ) | _2;
    }


// CLASS socket_address_info
class socket_address_info
    {
public:
    _Socket_address_flags_helper flags;
    socket_address_family family;
    socket_type socktype;
    socket_protocol protocol;
    std::string canonname;
    std::shared_ptr<_Socket_address_base> addr;

public:
    inline socket_address_info()
        : flags( 0 )
        , family( socket_address_family::unknown )
        , socktype( socket_type::unknown )
        , protocol( unknown_socket_protocol() )
        , canonname( "" )
        , addr( nullptr )
        {   // construct uninitialized socket address info
        }

    inline socket_address_info(
        socket_address_family _Family,
        socket_type _Type,
        socket_protocol _Protocol,
        _Socket_address_flags_helper _Flags = _Socket_address_flags_helper( 0 ) )
        : flags( _Flags )
        , family( _Family )
        , socktype( _Type )
        , protocol( _Protocol )
        , canonname( "" )
        , addr( nullptr )
        {   // construct socket address info hints structure
        }

    inline socket_address_info(
        socket_address_family _Family,
        socket_type _Type,
        socket_protocol _Protocol,
        std::string _Canonname,
        _Socket_address_flags_helper _Flags = _Socket_address_flags_helper( 0 ) )
        : flags( _Flags )
        , family( _Family )
        , socktype( _Type )
        , protocol( _Protocol )
        , canonname( _Canonname )
        , addr( nullptr )
        {   // construct socket address info hints structure
        }

    inline socket_address_info( const addrinfo& _Addrinfo )
        : flags( _Addrinfo.ai_flags )
        , family( static_cast<socket_address_family>(_Addrinfo.ai_family) )
        , socktype( static_cast<socket_type>(_Addrinfo.ai_socktype) )
        , protocol( static_cast<int>(_Addrinfo.ai_protocol) )
        , canonname( _Addrinfo.ai_canonname ? _Addrinfo.ai_canonname : "" )
        , addr( nullptr )
        {   // construct socket address info from platform-dependent addrinfo struct
        this->addr = _Create_socket_address( this->family, _Addrinfo.ai_addr );
        }

    _NODISCARD inline addrinfo get_addrinfo() const noexcept
        {   // cast socket_address_info into platform-dependent addrinfo structure
        _MyCanonname = nullptr;
        const size_t _canoname_len = canonname.length() + 1;
        if( _canoname_len > 1 )
            {
            _MyCanonname = std::shared_ptr<char>(
                new char[_canoname_len], _Array_delete<char> );
            // copy name to the temporary buffer
            __impl::memcpy(
                _MyCanonname.get(),
                canonname.c_str(),
                _canoname_len );
            }
        // prepare platform-dependent addrinfo structure
        addrinfo _addrinfo;
        __impl::memset( &_addrinfo, 0, sizeof( _addrinfo ) );
        _addrinfo.ai_family = static_cast<int>(family);
        _addrinfo.ai_socktype = static_cast<int>(socktype);
        _addrinfo.ai_protocol = static_cast<int>(protocol);
        _addrinfo.ai_flags = static_cast<int>(flags);
        _addrinfo.ai_canonname = _MyCanonname.get();
        return _addrinfo;
        }

private:
    mutable std::shared_ptr<char> _MyCanonname;
    };


_NODISCARD inline socket_address_info get_socket_address_info(
    const std::string& _Hostname,
    const std::string& _Svc_name,
    const socket_address_info& _Hints )
    {   // get socket address info using provided hints
    addrinfo* _addrinfo;
    addrinfo _hints = _Hints.get_addrinfo();
    const char* _pNodeName = !_Hostname.empty() ? _Hostname.c_str() : nullptr;
    const char* _pSvcName = !_Svc_name.empty() ? _Svc_name.c_str() : nullptr;
    if( __impl::getaddrinfo( _pNodeName, _pSvcName, &_hints, &_addrinfo ) != 0 )
        { // call to getaddrinfo failed
        throw socket_exception( -1 );
        }
    // pass retrieved pointer to shared_ptr for automatic memory management
    std::shared_ptr<addrinfo> _addrinfo_sp( _addrinfo, __impl::freeaddrinfo );
    return socket_address_info( *_addrinfo_sp );
    }


// ENUM CLASS dscp
enum class dscp
    {
    cs0         = 0b000'000, // standard
    cs1         = 0b001'000, // low-priority data
    cs2         = 0b010'000, // oam
    cs3         = 0b011'000, // broadcast video
    cs4         = 0b100'000, // real-time interactive
    cs5         = 0b101'000, // signaling
    cs6         = 0b110'000, // network control
    ef          = 0b101'110, // telephony
    af11        = 0b001'010, // high-throughput data (low drop probability)
    af12        = 0b001'100, // high-throughput data (medium drop probability)
    af13        = 0b001'110, // high-throughput data (high drop probability)
    af21        = 0b010'010, // low-latency data (low drop probability)
    af22        = 0b010'100, // low-latency data (medium drop probability)
    af23        = 0b010'110, // low-latency data (high drop probability)
    af31        = 0b011'010, // multimedia streaming (low drop probability)
    af32        = 0b011'100, // multimedia streaming (medium drop probability)
    af33        = 0b011'110, // multimedia streaming (high drop probability)
    af41        = 0b100'010, // multimedia conferencing (low drop probability)
    af42        = 0b100'100, // multimedia conferencing (medium drop probability)
    af43        = 0b100'110  // multimedia conferencing (high drop probability)
    };

typedef dscp differentiated_services_code_point;


// ENUM CLASS socket_send_flags
enum class inet_header_flags
    {
    none                = 0,
    more_fragments      = 1,
    dont_fragment       = 2
    };

using _Inet_header_flags_helper = _Socket_flags_helper<inet_header_flags, int>;

_NODISCARD _Inet_header_flags_helper operator|( inet_header_flags _1, inet_header_flags _2 ) noexcept
    {   // construct socket recv flags helper from two flags
    return _Inet_header_flags_helper( _1 ) | _2;
    }


// ENUM CLASS ecn
enum class ecn_mode
    {
    disabled            = 0,
    ect_0               = 2,
    ect_1               = 1,
    ce                  = 3
    };

typedef ecn_mode explicit_congestion_notification_mode;


// CLASS inet_header
class inet_header
    {
protected:
    // header data, stored in big-endian dwords
    unsigned long _MyData[5];

public:
    inline inet_header() noexcept
        {   // construct empty ipv4 header
        __impl::memset( this->_MyData, 0, sizeof( _MyData ) );
        _MyData[0] = make_big_endian( 0x45000014UL ).as_big_endian();
        }

    inline inet_header( const void* _Packet, size_t _PacketSize )
        {   // construct ipv4 header from packet
        if( _PacketSize < (5 * sizeof( unsigned long )) )
            { // provided data does not contain valid ipv4 packet header
            throw std::invalid_argument(
                "IPv4 header must be at least 20 bytes long." );
            }
        const inet_header* pPacket = reinterpret_cast<const inet_header*>(_Packet);
        if( pPacket->header_length() < 5 )
            { // provided data is not valid ipv4 packet header
            throw std::invalid_argument(
                "IPv4 header IHL (Internet Header Length) must be at least 5. "
                "The provided data is not valid IPv4 header." );
            }
        _Copy_from( _Packet );
        }

    inline inet_header( const void* _Packet, size_t _PacketSize, std::nothrow_t ) noexcept
        {   // construct ipv4 header from packet
        __impl::memset( this->_MyData, 0, sizeof( _MyData ) );
        if( _PacketSize > 0 )
            __impl::memcpy( this->_MyData, _Packet, __impl::min( _PacketSize, sizeof( _MyData ) ) );
        }

    inline inet_header( const inet_header& _Other ) noexcept
        {   // construct copy of ipv4 header
        _Copy_from( _Other._MyData );
        }

    inline inet_header& operator=( const inet_header& _Other ) noexcept
        {   // assign values from other ipv4 header
        _Copy_from( _Other._MyData );
        return (*this);
        }

    inline inet_header( inet_header&& _Other ) noexcept
        : inet_header()
        {   // move header from other
        __impl::swap( this->_MyData, _Other._MyData );
        }

    inline inet_header& operator=( inet_header&& _Other ) noexcept
        {   // move header from other
        inet_header _Head( __impl::move( _Other ) );
        __impl::swap( this->_MyData, _Head._MyData );
        return (*this);
        }

    _NODISCARD inline void* data() noexcept
        {   // get pointer to the ipv4 header raw data
        return reinterpret_cast<void*>(this);
        }

    _NODISCARD inline const void* data() const noexcept
        {   // get pointer to the ipv4 header raw data
        return reinterpret_cast<const void*>(this);
        }

    _NODISCARD inline size_t size() const noexcept
        {   // get size of the ipv4 header
        return sizeof( inet_header );
        }

    _NODISCARD inline unsigned long version() const noexcept
        {   // get header version
        return static_cast<unsigned long>(((make_big_endian( _MyData[0], true ).as_host_endian()) >> 28) & 0xF);
        }

    inline inet_header& version( unsigned long _Ver ) noexcept
        {   // set header version
        _MyData[0] &= make_big_endian( 0x0FFFFFFF );
        _MyData[0] |= make_big_endian( (_Ver << 28) & 0xF0000000 );
        return (*this);
        }

    _NODISCARD inline unsigned long header_length() const noexcept
        {   // get length of the header
        return static_cast<unsigned long>(((make_big_endian( _MyData[0], true ).as_host_endian()) >> 24) & 0xF);
        }

    inline inet_header& header_length( unsigned long _Length ) noexcept
        {   // set length of the header
        _MyData[0] &= make_big_endian( 0xF0FFFFFF );
        _MyData[0] |= make_big_endian( (_Length << 24) & 0x0F000000 );
        return (*this);
        }

    _NODISCARD inline dscp type_of_service() const noexcept
        {   // get type of service
        return static_cast<dscp>(((make_big_endian( _MyData[0], true ).as_host_endian()) >> 18) & 0x3F);
        }

    inline inet_header& type_of_service( dscp _Type ) noexcept
        {   // set type of service
        _MyData[0] &= make_big_endian( 0xFF03FFFF );
        _MyData[0] |= make_big_endian( (static_cast<unsigned long>(_Type) << 18) & 0x00FC0000 );
        return (*this);
        }

    _NODISCARD inline ecn_mode ecn() const noexcept
        {   // get ecn
        return static_cast<ecn_mode>(((make_big_endian( _MyData[0], true ).as_host_endian()) >> 16) & 0x3);
        }

    inline inet_header& ecn( ecn_mode _Ecn ) noexcept
        {   // set ecn
        _MyData[0] &= make_big_endian( 0xFFFCFFFF );
        _MyData[0] |= make_big_endian( (static_cast<unsigned long>(_Ecn) << 16) & 0x00030000 );
        return (*this);
        }

    _NODISCARD inline unsigned long packet_length() const noexcept
        {   // get total packet length
        return static_cast<unsigned long>(((make_big_endian( _MyData[0], true ).as_host_endian())) & 0xFFFF);
        }

    inline inet_header& packet_length( unsigned long _Length ) noexcept
        {   // set total packet length
        _MyData[0] &= make_big_endian( 0xFFFF0000 );
        _MyData[0] |= make_big_endian( _Length & 0x0000FFFF );
        return (*this);
        }

    _NODISCARD inline unsigned long identification() const noexcept
        {   // get packet identification number
        return static_cast<unsigned long>(((make_big_endian( _MyData[1], true ).as_host_endian()) >> 16) & 0xFFFF);
        }

    inline inet_header& identification( unsigned long _Id ) noexcept
        {   // set packet identification number
        _MyData[1] &= make_big_endian( 0x0000FFFF );
        _MyData[1] |= make_big_endian( (_Id << 16) & 0xFFFF0000 );
        return (*this);
        }

    _NODISCARD inline _Inet_header_flags_helper flags() const noexcept
        {   // get packet flags
        return static_cast<_Inet_header_flags_helper>(((make_big_endian( _MyData[1], true ).as_host_endian()) >> 13) & 0x7);
        }

    inline inet_header& flags( _Inet_header_flags_helper _Flags ) noexcept
        {   // set packet flags
        _MyData[1] &= make_big_endian( 0xFFFF1FFF );
        _MyData[1] |= make_big_endian( (static_cast<unsigned long>((int)_Flags) << 13) & 0x0000E000 );
        return (*this);
        }

    _NODISCARD inline unsigned long fragment_offset() const noexcept
        {   // get fragment offset
        return static_cast<unsigned long>(make_big_endian( _MyData[1], true ).as_host_endian() & 0x00001FFF);
        }

    inline inet_header& fragment_offset( unsigned long _FragOffset ) noexcept
        {   // set fragment offset
        _MyData[1] &= make_big_endian( 0xFFFFE000 );
        _MyData[1] |= make_big_endian( _FragOffset & 0x00001FFF );
        return (*this);
        }

    _NODISCARD inline unsigned char ttl() const noexcept
        {   // get packet TTL (time-to-live) value
        return static_cast<unsigned long>(((make_big_endian( _MyData[2], true ).as_host_endian()) >> 24) & 0xFF);
        }

    inline inet_header& ttl( unsigned char _Ttl ) noexcept
        {   // set packet TTL (time-to-live) value
        _MyData[2] &= make_big_endian( 0x00FFFFFF );
        _MyData[2] |= make_big_endian( (static_cast<unsigned long>(_Ttl) << 24) & 0xFF000000 );
        return (*this);
        }

    _NODISCARD inline socket_protocol protocol() const noexcept
        {   // get transport layer protocol
        return socket_protocol( static_cast<int>(((make_big_endian( _MyData[2], true ).as_host_endian()) >> 16) & 0xFF) );
        }

    inline inet_header& protocol( socket_protocol _Proto ) noexcept
        {   // set transport layer protocol
        _MyData[2] &= make_big_endian( 0xFF00FFFF );
        _MyData[2] |= make_big_endian( (static_cast<unsigned long>(_Proto.get_id()) << 16) & 0x00FF0000 );
        return (*this);
        }

    _NODISCARD inline unsigned short checksum() const noexcept
        {   // get header checksum
        return static_cast<unsigned long>(make_big_endian( _MyData[2], true ).as_host_endian() & 0x0000FFFF);
        }

    inline inet_header& checksum( unsigned short _Checksum ) noexcept
        {   // set header checksum
        _MyData[2] &= make_big_endian( 0xFFFF0000 );
        _MyData[2] |= make_big_endian<unsigned long>( static_cast<unsigned long>(_Checksum) & 0x0000FFFF );
        return (*this);
        }

    _NODISCARD inline unsigned long source_ip_address() const noexcept
        {   // get source ip address in host byte order
        return make_big_endian( _MyData[3], true ).as_host_endian();
        }

    inline inet_header& source_ip_address( unsigned long _Addr ) noexcept
        {   // set source ip address
        _MyData[3] = make_big_endian( _Addr );
        return (*this);
        }

    _NODISCARD inline unsigned long dest_ip_address() const noexcept
        {   // get destination ip address in host byte order
        return make_big_endian( _MyData[4], true ).as_host_endian();
        }

    inline inet_header& dest_ip_address( unsigned long _Addr ) noexcept
        {   // set destination ip address
        _MyData[4] = make_big_endian( _Addr );
        return (*this);
        }

protected:
    inline void _Copy_from( const void* _Ptr ) noexcept
        {   // copy values from _Ptr into _MyData
        __impl::memcpy( this->_MyData, _Ptr, sizeof( _MyData ) );
        }
    };


// ENUM CLASS socket_recv_flags
enum class socket_recv_flags
    {
    none                = 0,
    oob                 = MSG_OOB,
    peek                = MSG_PEEK,
    trunc               = MSG_TRUNC,
    wait_all            = MSG_WAITALL
    };

using _Socket_recv_flags_helper = _Socket_flags_helper<socket_recv_flags, int>;

_NODISCARD _Socket_recv_flags_helper operator|( socket_recv_flags _1, socket_recv_flags _2 ) noexcept
    {   // construct socket recv flags helper from two flags
    return _Socket_recv_flags_helper( _1 ) | _2;
    }


// ENUM CLASS socket_send_flags
enum class socket_send_flags
    {
    none                = 0,
    dont_route          = MSG_DONTROUTE,
    oob                 = MSG_OOB
    };

using _Socket_send_flags_helper = _Socket_flags_helper<socket_send_flags, int>;

_NODISCARD _Socket_send_flags_helper operator|( socket_send_flags _1, socket_send_flags _2 ) noexcept
    {   // construct socket recv flags helper from two flags
    return _Socket_send_flags_helper( _1 ) | _2;
    }


// CLASS socket
class socket
    {
public: // flag bits
    static constexpr int in = 1;
    static constexpr int out = 2;

private:
    static constexpr int _inout = socket::in | socket::out;

public:
    socket( const socket& ) = delete;

    inline socket()
        : _MyHandle( _Invalid_socket )
        , _MyAddr_family( socket_address_family::unknown )
        , _MyType( socket_type::unknown )
        , _MyProtocol( unknown_socket_protocol() )
        {   // construct uninitialized socket
        }

    inline socket( socket_address_family _Family, socket_type _Type, socket_protocol _Protocol )
        : _MyHandle( _Invalid_socket )
        , _MyAddr_family( _Family )
        , _MyType( _Type )
        , _MyProtocol( _Protocol )
        {   // construct socket object
        this->_MyHandle = _Throw_if_failed( __impl::socket(
            static_cast<int>(_Family),
            static_cast<int>(_Type),
            static_cast<int>(_Protocol) ) );
        }

    inline socket( const socket_address_info& _Addrinfo )
        : socket( _Addrinfo.family, _Addrinfo.socktype, _Addrinfo.protocol )
        {   // construct socket object from addrinfo structure
        this->_MyAddrinfo.reset( new socket_address_info( _Addrinfo ) );
        }

    inline socket( socket&& _Original ) noexcept
        : socket()
        {   // take ownership of socket object
        swap( _Original );
        }

    inline socket& operator=( socket&& _Original ) noexcept
        {   // take ownership of socket object
        _Close();
        swap( _Original );
        return (*this);
        }

    inline virtual ~socket() noexcept
        {   // destroy socket object
        _Close();
        }

    inline void swap( socket& _Other ) noexcept
        {   // exchange sockets
        __impl::swap( _MyHandle, _Other._MyHandle );
        __impl::swap( _MyAddr_family, _Other._MyAddr_family );
        __impl::swap( _MyType, _Other._MyType );
        __impl::swap( _MyProtocol, _Other._MyProtocol );
        }

    template<typename _SockOptTy>
    inline void set_opt( _SockOptTy _Opt, const void* _Optval, size_t _Optlen )
        {   // set socket option value
        _Set_socket_opt( static_cast<int>(_Opt), _Socket_opt_level<_SockOptTy>::value,
            _Optval, _Optlen );
        }

    template<typename _SockOptTy, typename _VTy>
    inline void set_opt( _SockOptTy _Opt, const _VTy& _Optval )
        {   // set socket option value
        set_opt( _Opt, &_Optval, sizeof( _VTy ) );
        }

    template<typename _SockOptTy>
    inline void set_opt( _SockOptTy _Opt, const bool& _Optval )
        {   // set socket option value
        set_opt( _Opt, static_cast<long>(_Optval) );
        }

    template<typename _SockOptTy>
    inline void get_opt( _SockOptTy _Opt, void* _Optval, size_t* _Optlen ) const
        {   // get socket option value
        _Get_socket_opt( static_cast<int>(_Opt), _Socket_opt_level<_SockOptTy>::value,
            _Optval, _Optlen );
        }

    template<typename _SockOptTy, typename _VTy>
    inline void get_opt( _SockOptTy _Opt, _VTy& _Optval ) const
        {   // get socket option value
        size_t optsize = sizeof( _VTy );
        get_opt( _Opt, &_Optval, &optsize );
        }

    template<typename _SockOptTy>
    inline void get_opt( _SockOptTy _Opt, bool& _Optval ) const
        {   // get socket option value
        size_t optsize = sizeof( long );
        long optval = 0;
        get_opt( _Opt, &optval, &optsize );
        _Optval = static_cast<bool>(optval);
        }

    inline virtual int send( const void* _Data, size_t _ByteSize, _Socket_send_flags_helper _Flags = socket_send_flags::none )
        {   // send message to the remote host
        return _Throw_if_failed( (int)__impl::send( this->_MyHandle,
            reinterpret_cast<const _Sockcomm_data_t*>(_Data),
            static_cast<_Sockcomm_data_size_t>(_ByteSize),
            static_cast<int>(_Flags) ) );
        }

    template<typename _SockAddrTy>
    inline int send_to( const void* _Data, size_t _ByteSize, const _SockAddrTy* _Addr, size_t _Addrlen,
            _Socket_send_flags_helper _Flags = socket_send_flags::none )
        {   // send message to the remote host
        return _Throw_if_failed( (int)__impl::sendto( this->_MyHandle,
            reinterpret_cast<const _Sockcomm_data_t*>(_Data),
            static_cast<_Sockcomm_data_size_t>(_ByteSize),
            static_cast<int>(_Flags),
            reinterpret_cast<const sockaddr*>(_Addr),
            static_cast<_Sock_size_t>(_Addrlen) ) );
        }

    inline virtual int recv( void* _Data, size_t _ByteSize, _Socket_recv_flags_helper _Flags = socket_recv_flags::none )
        {   // receive message from the remote host
        return _Throw_if_failed( (int)__impl::recv( this->_MyHandle,
            reinterpret_cast<_Sockcomm_data_t*>(_Data),
            static_cast<_Sockcomm_data_size_t>(_ByteSize),
            static_cast<int>(_Flags) ) );
        }

    template<typename _SockAddrTy>
    inline int recv_from( void* _Data, size_t _ByteSize, _SockAddrTy* _Addr, size_t* _Addrlen,
            _Socket_recv_flags_helper _Flags = socket_recv_flags::none )
        {   // receive message from the remote host
        // Length of _Addrlen value may differ, change it to platform-dependent
        // for the call and then cast it to size_t.
        _Sock_size_t addrlen = _Static_optional_or_default<_Sock_size_t>( _Addrlen, 0 );
        int receivedByteCount = _Throw_if_failed( (int)__impl::recvfrom( this->_MyHandle,
            reinterpret_cast<_Sockcomm_data_t*>(_Data),
            static_cast<_Sockcomm_data_size_t>(_ByteSize),
            static_cast<int>(_Flags),
            reinterpret_cast<sockaddr*>(_Addr),
            reinterpret_cast<_Sock_size_t*>((_Addrlen) ? &addrlen : nullptr) ) );
        if( _Addrlen != nullptr )
            { // Pass retrieved addrlen to the actual output parameter
            (*_Addrlen) = static_cast<size_t>(addrlen);
            }
        return receivedByteCount;
        }

    template<typename _SockAddrTy>
    inline void bind( const _SockAddrTy* _Addr, size_t _Addrlen )
        {   // bind socket to the network interface
        _Throw_if_failed( __impl::bind( this->_MyHandle,
            reinterpret_cast<const sockaddr*>(_Addr),
            static_cast<_Sock_size_t>(_Addrlen) ) );
        }

    inline void bind( const _Socket_address_base& _Addr )
        {   // bind socket to the network interface
        return bind( _Addr.get_native_sockaddr(),
            _Addr.get_native_sockaddr_size() );
        }

    inline void bind()
        {   // bind socket to the network interface
        if( this->_MyAddrinfo == nullptr )
            {
            throw std::runtime_error(
                "Argumentless binding is available only if socket has been constructed "
                "with socket_address_info structure" );
            }
        return bind( _MyAddrinfo->addr->get_native_sockaddr(),
            _MyAddrinfo->addr->get_native_sockaddr_size() );
        }

    inline void listen( size_t _QueueLength = SOMAXCONN )
        {   // start listening for incoming connections
        _Throw_if_failed( __impl::listen( this->_MyHandle,
            static_cast<int>(_QueueLength) ) );
        }

    template<typename _SockAddrTy>
    inline void connect( const _SockAddrTy* _Addr, size_t _Addrlen )
        {   // connect to the remote host
        _Throw_if_failed( __impl::connect( this->_MyHandle,
            reinterpret_cast<const sockaddr*>(_Addr),
            static_cast<_Sock_size_t>(_Addrlen) ) );
        }

    inline void connect( const _Socket_address_base& _Addr )
        {   // connect to the remote host
        return connect( _Addr.get_native_sockaddr(),
            _Addr.get_native_sockaddr_size() );
        }

    _NODISCARD inline socket accept()
        {   // accept incoming connection from the client
        return accept<sockaddr>( nullptr, nullptr );
        }

    template<typename _SockAddrTy>
    _NODISCARD inline socket accept( _SockAddrTy* _Addr, size_t* _Addrlen )
        {   // accept incoming connection from the client
        // Length of _Addrlen value may differ, change it to platform-dependent
        // for the call and then cast it to size_t.
        _Sock_size_t addrlen = _Static_optional_or_default<_Sock_size_t>( _Addrlen, 0 );
        _Socket_handle _Accepted_handle = _Throw_if_failed( __impl::accept( this->_MyHandle,
            reinterpret_cast<sockaddr*>(_Addr),
            reinterpret_cast<_Sock_size_t*>((_Addrlen) ? &addrlen : nullptr) ) );
        if( _Addrlen != nullptr )
            { // Pass retrieved addrlen to the actual output parameter
            (*_Addrlen) = static_cast<size_t>(addrlen);
            }
        return socket( static_cast<_Socket_handle>(_Accepted_handle),
            this->_MyAddr_family, this->_MyType, this->_MyProtocol );
        }

    inline void shutdown( int _Flags = socket::_inout )
        {   // close socket connection in specified direction
        int how = 0;
        if( (_Flags & socket::_inout) == socket::_inout )
            how = socket::_Shut_inout;
        else if( (_Flags & socket::in) == socket::in )
            how = socket::_Shut_in;
        else if( (_Flags & socket::out) == socket::out )
            how = socket::_Shut_out;
        _Throw_if_failed( __impl::shutdown( this->_MyHandle, how ) );
        }

public:
    _NODISCARD inline _Socket_handle get_native_handle() const noexcept
        {   // retrieve native socket handle
        return this->_MyHandle;
        }

    _NODISCARD inline socket_address_family get_address_family() const noexcept
        {   // get socket address family
        return this->_MyAddr_family;
        }

    _NODISCARD inline socket_type get_socket_type() const noexcept
        {   // get socket type
        return this->_MyType;
        }

    _NODISCARD inline socket_protocol get_protocol() const noexcept
        {   // get socket protocol
        return this->_MyProtocol;
        }

protected:
    _Socket_handle _MyHandle;
    socket_address_family _MyAddr_family;
    socket_type _MyType;
    socket_protocol _MyProtocol;

    std::shared_ptr<socket_address_info> _MyAddrinfo;

    inline socket( _Socket_handle _Handle, socket_address_family _Family, socket_type _Type, socket_protocol _Protocol ) noexcept
        : _MyHandle( _Handle )
        , _MyAddr_family( _Family )
        , _MyType( _Type )
        , _MyProtocol( _Protocol )
        {   // construct socket object from existing handle
        }

    inline void _Close() noexcept
        {   // close the socket handle
        __impl::closesocket( this->_MyHandle );
        this->_MyHandle = _Invalid_socket;
        this->_MyAddr_family = socket_address_family::unknown;
        this->_MyType = socket_type::unknown;
        this->_MyProtocol = unknown_socket_protocol();
        }

    _NODISCARD inline bool _Is_stream_socket() const noexcept
        {   // checks if the socket is reliable, stream one
        return (this->_MyType == socket_type::stream)
            || (this->_MyType == socket_type::seqpacket);
        }

    template<typename _Ty>
    inline _Ty& _Throw_if_failed( _Ty&& _Retval ) const
        {   // throw exception if _Retval indicates error
        if( reinterpret_cast<const int&>(_Retval) < 0 )
            { // assume that all negative return values indicate error
            throw socket_exception( __impl::geterror(
                static_cast<int>(_Retval) ) );
            }
        return _Retval;
        }

private:
    inline virtual void _Set_socket_opt( int _Opt, int _Opt_level, const void* _Optval, size_t _Optlen )
        {   // set socket option value
        _LIBSOCK_CHECK_ARG_NOT_NULL( _Optval );
        _LIBSOCK_CHECK_ARG_NOT_EQ( _Optlen, 0 );
        _Adjust_socket_opt_level( _Opt_level );
        _Throw_if_failed( __impl::setsockopt( this->_MyHandle,
            _Opt_level, _Opt,
            reinterpret_cast<const _Sockopt_data_t*>(_Optval),
            static_cast<_Sock_size_t>(_Optlen) ) );
        }

    inline virtual void _Get_socket_opt( int _Opt, int _Opt_level, void* _Optval, size_t* _Optlen ) const
        {   // get socket option value
        _LIBSOCK_CHECK_ARG_NOT_NULL( _Optval );
        _LIBSOCK_CHECK_ARG_NOT_NULL( _Optlen );
        _LIBSOCK_CHECK_ARG_NOT_EQ( *_Optlen, 0 );
        _Adjust_socket_opt_level( _Opt_level );
        _Sock_size_t optlen = _Static_optional_or_default<_Sock_size_t>( _Optlen, 0 );
        _Throw_if_failed( __impl::getsockopt( this->_MyHandle,
            _Opt_level, _Opt,
            reinterpret_cast<_Sockopt_data_t*>(_Optval),
            reinterpret_cast<_Sock_size_t*>(_Optlen ? &optlen : nullptr) ) );
        if( _Optlen != nullptr )
            (*_Optlen) = static_cast<size_t>(optlen);
        }

    inline virtual void _Adjust_socket_opt_level( int& _Opt_level ) const noexcept
        {   // adjust socket option level to socket's address family
        if( _Opt_level == _Socket_opt_level<socket_opt_ip>::value && _MyAddr_family == socket_address_family::inet6 )
            { // _Socket_opt_level for IP has additional value for IPv6
            _Opt_level = _Socket_opt_level<socket_opt_ip>::value_v6;
            }
        }

private: // platform-dependent shutdown values
#if defined( OS_WINDOWS )
    static constexpr int _Shut_in = SD_RECEIVE;
    static constexpr int _Shut_out = SD_SEND;
    static constexpr int _Shut_inout = SD_BOTH;
#elif defined( OS_LINUX )
    static constexpr int _Shut_in = SHUT_RD;
    static constexpr int _Shut_out = SHUT_WR;
    static constexpr int _Shut_inout = SHUT_RDWR;
#else
#error Shutdown flags not defined
#endif

    template<typename _Elem, typename _Traits>
    friend class basic_socketstream;
    };

template<>
inline void socket::set_opt( socket_opt_ip _Opt, const void* _Optval, size_t _Optlen )
    {   // set socket ip option value specialization
#if defined( OS_LINUX )
    if( _Opt == socket_opt_ip::dont_fragment )
        {   // Linux OSes get/set DF flag via mtu MTU_DISCOVER settings
        if( _Optlen != sizeof( long ) )
            throw std::invalid_argument( "dont_fragment option requires LONG argument" );
        long value = _Reinterpret_optional_or_default( _Optval, 0 );
        value = (value != 0) ? IP_PMTUDISC_DO : IP_PMTUDISC_DONT;
        return _Set_socket_opt( static_cast<int>(socket_opt_ip::mtu_discover), _Socket_opt_level<socket_opt_ip>::value,
            &value, sizeof( value ) );
        }
#endif// OS_LINUX
    return _Set_socket_opt( static_cast<int>(_Opt), _Socket_opt_level<socket_opt_ip>::value,
        _Optval, _Optlen );
    }

template<>
inline void socket::get_opt( socket_opt_ip _Opt, void* _Optval, size_t* _Optlen ) const
    {   // get socket ip option value specialization
#if defined( OS_LINUX )
    if( _Opt == socket_opt_ip::dont_fragment )
        {   // Linux OSes get/set DF flag via mtu MTU_DISCOVER settings
        if( (!_Optlen) || (*_Optlen) != sizeof( long ) )
            throw std::invalid_argument( "dont_fragment option requires LONG argument" );
        long value = 0;
        _Get_socket_opt( static_cast<int>(socket_opt_ip::mtu_discover), _Socket_opt_level<socket_opt_ip>::value,
            &value, _Optlen );
        if( _Optval != nullptr )
            * reinterpret_cast<long*>(_Optval) = (value == IP_PMTUDISC_DONT) ? 0 : 1;
        }
#endif// OS_LINUX
    return _Get_socket_opt( static_cast<int>(_Opt), _Socket_opt_level<socket_opt_ip>::value,
        _Optval, _Optlen );
    }


inline void swap( socket& _Left, socket& _Right ) noexcept
    {	// exchange values stored at _Left and _Right
    _Left.swap( _Right );
    }


// CLASS socketstream
template<typename _Elem, typename _Traits = std::char_traits<_Elem>>
class basic_socketstream
    : public std::ios_base
    {
public:
    static constexpr int text = 0;
    static constexpr int binary = 1;

private:
    static constexpr int _mode_default = basic_socketstream::binary;

public:
    typedef std::ios_base _MyBase;

    inline basic_socketstream() noexcept
        : _MyBase()
        , _MySocket()
        , _MyMode( basic_socketstream::_mode_default )
        {   // construct uninitialized socket stream
        }

    inline basic_socketstream( socket& _Socket, int _Mode = basic_socketstream::_mode_default )
        : _MySocket( &_Socket )
        , _MyMode( _Mode )
        {   // construct socket stream from socket object
        if( !this->_MySocket->_Is_stream_socket() )
            {
            throw std::invalid_argument( "cannot create stream from non-stream socket" );
            }
        }

    inline void swap( basic_socketstream& _Other )
        {   // exchange socket streams
        __impl::swap( this->_MySocket, _Other._MySocket );
        __impl::swap( this->_MyMode, _Other._MyMode );
        ios_base::swap( _Other );
        }
    
    inline basic_socketstream& operator<<( std::ios_base& (&_Mod)(std::ios_base&) )
        {   // set format flag
        _Mod( *this );
        return (*this);
        }

    inline basic_socketstream& operator<<( short _Val )
        { // send 16-bit signed integer
        return _Common_send_arithmetic( _Val );
        }

    inline basic_socketstream& operator<<( unsigned short _Val )
        { // send 16-bit unsigned integer
        return _Common_send_arithmetic( _Val );
        }

    inline basic_socketstream& operator<<( long _Val )
        { // send 32-bit signed integer
        return _Common_send_arithmetic( _Val );
        }

    inline basic_socketstream& operator<<( unsigned long _Val )
        { // send 32-bit unsigned integer
        return _Common_send_arithmetic( _Val );
        }

    inline basic_socketstream& operator<<( long long _Val )
        { // send 64-bit signed integer
        return _Common_send_arithmetic( _Val );
        }

    inline basic_socketstream& operator<<( unsigned long long _Val )
        { // send 64-bit unsigned integer
        return _Common_send_arithmetic( _Val );
        }

    inline basic_socketstream& operator<<( float _Val )
        { // send 32-bit floating-point
        return _Common_send_arithmetic( _Val );
        }

    inline basic_socketstream& operator<<( double _Val )
        { // send 64-bit floating point
        return _Common_send_arithmetic( _Val );
        }

    inline basic_socketstream& operator<<( const std::basic_string<_Elem, _Traits>& _Val )
        {   // send string
        _Throw_if_uninitialized();
        const _Elem* _Val_buffer = _Val.c_str();
        const size_t _Val_buffer_size = sizeof( _Elem ) * (_Val.length() + 1);
        this->_MySocket->send( _Val_buffer, _Val_buffer_size );
        return (*this);
        }

    inline basic_socketstream& operator<<( const _Elem* _Str )
        {   // send wide C-style string
        _Throw_if_uninitialized();
        const size_t _Str_size = sizeof( _Elem );
        if _CONSTEXPR_IF( sizeof( _Elem ) == sizeof( wchar_t ) )
            _Str_size *= __impl::wcslen( _Str ) + 1;
        else if _CONSTEXPR_IF( sizeof( _Elem ) == sizeof( char ) )
            _Str_size *= __impl::strlen( _Str ) + 1;
        else throw std::runtime_error( "unsupported char type" );
        this->_MySocket->send( _Str, _Str_size );
        return (*this);
        }

    template<size_t _Size>
    inline basic_socketstream& operator<<( const _Elem (&_Str)[_Size] )
        {   // send C-style string
        _Throw_if_uninitialized();
        this->_MySocket->send( _Str, _Size * sizeof( _Elem ) );
        return (*this);
        }

    inline basic_socketstream& operator>>( std::ios_base& (&_Mod)(std::ios_base&) )
        {   // set format flag
        _Mod( *this );
        return (*this);
        }

    inline basic_socketstream& operator>>( short& _Val )
        { // receive 16-bit signed integer
        return _Common_recv_arithmetic( _Val );
        }

    inline basic_socketstream& operator>>( unsigned short& _Val )
        { // receive 16-bit unsigned integer
        return _Common_recv_arithmetic( _Val );
        }

    inline basic_socketstream& operator>>( long& _Val )
        { // receive 32-bit signed integer
        return _Common_recv_arithmetic( _Val );
        }

    inline basic_socketstream& operator>>( unsigned long& _Val )
        { // receive 32-bit unsigned integer
        return _Common_recv_arithmetic( _Val );
        }

    inline basic_socketstream& operator>>( long long& _Val )
        { // receive 64-bit signed integer
        return _Common_recv_arithmetic( _Val );
        }

    inline basic_socketstream& operator>>( unsigned long long& _Val )
        { // receive 64-bit unsigned integer
        return _Common_recv_arithmetic( _Val );
        }

    inline basic_socketstream& operator>>( float& _Val )
        { // receive 32-bit floating-point
        return _Common_recv_arithmetic( _Val );
        }

    inline basic_socketstream& operator>>( double& _Val )
        { // receive 64-bit floating point
        return _Common_recv_arithmetic( _Val );
        }

    inline basic_socketstream& operator>>( std::basic_string<_Elem, _Traits>& _Val )
        {   // receive string
        return _Common_recv_string( _Val );
        }

    template<size_t _Size>
    inline basic_socketstream& operator>>( _Elem (&_Str)[_Size] )
        {   // receive wide C-style string
        std::basic_string<_Elem, _Traits> _Str_buffer;
        _Common_recv_string( _Str_buffer, _Size );
        if _CONSTEXPR_IF( sizeof( _Elem ) == sizeof( wchar_t ) )
            {
            __impl::wcscpy(
                reinterpret_cast<_Elem[_Size]>(_Str),
                reinterpret_cast<const _Elem*>(_Str_buffer.c_str()) );
            }
        else if _CONSTEXPR_IF( sizeof( _Elem ) == sizeof( char ) )
            {
            __impl::strcpy(
                reinterpret_cast<_Elem[_Size]>(_Str),
                reinterpret_cast<const _Elem*>(_Str_buffer.c_str()) );
            }
        else throw std::runtime_error( "unsupported char type" );
        return (*this);
        }


protected:
    socket* _MySocket;
    int _MyMode;

    inline void _Throw_if_uninitialized()
        {   // throw an exception if the stream has not been initialized
        if( this->_MySocket == nullptr )
            {
            throw std::runtime_error( "stream is not associated with any socket" );
            }
        }

    template<typename _Ty>
    inline basic_socketstream& _Common_send_arithmetic( const _Ty& _Val,
            typename std::enable_if<std::is_arithmetic<_Ty>::value>::type* = nullptr )
        {   // serialize arithmetic value
        _Throw_if_uninitialized();
        if( (this->_MyMode & basic_socketstream::binary) == basic_socketstream::binary )
            { // binary serialization, send raw bytes
            this->_MySocket->send( &_Val, sizeof( _Ty ) );
            }
        else
            { // text serialization, send string representation
            std::basic_string<_Elem, _Traits> _Val_str = (_Create_stringstream() << _Val).str();
            this->_MySocket->send( _Val_str.c_str(), _Val_str.length() + 1 );
            }
        return (*this);
        }

    template<typename _Ty>
    inline basic_socketstream& _Common_recv_arithmetic( _Ty& _Val,
            typename std::enable_if<std::is_arithmetic<_Ty>::value>::type* = nullptr )
        {   // deserialize arithmetic value
        _Throw_if_uninitialized();
        if( (this->_MyMode & basic_socketstream::binary) == basic_socketstream::binary )
            { // binary deserialization, recv raw bytes
            this->_MySocket->recv( &_Val, sizeof( _Ty ) );
            }
        else
            { // text deserialization, recv string representation
            std::basic_string<_Elem, _Traits> _Val_str;
            _Common_recv_string( _Val_str );
            _Create_stringstream( _Val_str ) >> _Val;
            }
        return (*this);
        }

    inline basic_socketstream& _Common_recv_string( std::basic_string<_Elem, _Traits>& _Str,
            size_t _Maxlen = std::numeric_limits<size_t>::max() )
        {   // receive string value
        _Throw_if_uninitialized();
        std::basic_stringstream<_Elem, _Traits> _Str_stream;
        std::vector<_Elem> _Str_buffer( 1024 );
        bool _Has_end = false;
        while( !_Has_end )
            { // read stream until terminator is found
            _Elem* _Str_buffer_ptr = _Str_buffer.data();
            size_t _Str_buffer_len = _Str_buffer.size() - 1;
            size_t _Str_buffer_size = _Str_buffer_len * sizeof( _Elem );
            // dont remove message from the queue yet
            this->_MySocket->recv( _Str_buffer_ptr, _Str_buffer_size, socket_recv_flags::peek );
            _Str_buffer_ptr[_Str_buffer_len] = 0;
            const _Elem* p = _Str_buffer_ptr;
            while( ((*p) != (_Elem)(0)) ) ++p;
            if( p != _Str_buffer_ptr + _Str_buffer_len )
                { // end of string found, add terminator
                _Str_buffer.resize( static_cast<size_t>(p - _Str_buffer_ptr) / sizeof( _Elem ) + 1 );
                _Has_end = true;
                }
            else _Str_buffer.resize( _Str_buffer.size() + 1024 );
            }
        if( _Str_buffer.size() > _Maxlen )
            { // insufficient buffer
            throw std::runtime_error( "insufficient buffer for string" );
            }
        // remove whole string from the queue
        this->_MySocket->recv( _Str_buffer.data(), _Str_buffer.size() * sizeof( _Elem ) );
        _Str.assign( _Str_buffer.data() );
        return (*this);
        }

    inline std::basic_ostringstream<_Elem, _Traits> _Create_stringstream()
        {   // construct stringstream with the same formatting as the socketstream
        std::basic_ostringstream<_Elem, _Traits> _Stream;
        const int _fmtfl = setf( 0 );
        if( _fmtfl & _MyBase::dec ) _Stream << std::dec;
        if( _fmtfl & _MyBase::oct ) _Stream << std::oct;
        if( _fmtfl & _MyBase::hex ) _Stream << std::hex;
        if( _fmtfl & _MyBase::boolalpha ) _Stream << std::boolalpha;
        if( _fmtfl & _MyBase::fixed ) _Stream << std::fixed;
        if( _fmtfl & _MyBase::internal ) _Stream << std::internal;
        if( _fmtfl & _MyBase::left ) _Stream << std::left;
        if( _fmtfl & _MyBase::right ) _Stream << std::right;
        if( _fmtfl & _MyBase::scientific ) _Stream << std::scientific;
        if( _fmtfl & _MyBase::showbase ) _Stream << std::showbase;
        if( _fmtfl & _MyBase::showpoint ) _Stream << std::showpoint;
        if( _fmtfl & _MyBase::showpos ) _Stream << std::showpos;
        if( _fmtfl & _MyBase::skipws ) _Stream << std::skipws;
        if( _fmtfl & _MyBase::unitbuf ) _Stream << std::unitbuf;
        if( _fmtfl & _MyBase::uppercase ) _Stream << std::uppercase;
        return _Stream;
        }

    inline std::basic_istringstream<_Elem, _Traits> _Create_stringstream( const std::basic_string<_Elem, _Traits>& _Str )
        {   // construct stringstream with the same formatting as the socketstream
        std::basic_istringstream<_Elem, _Traits> _Stream( _Str );
        const int _fmtfl = setf( 0 );
        if( _fmtfl & _MyBase::dec ) _Stream >> std::dec;
        if( _fmtfl & _MyBase::oct ) _Stream >> std::oct;
        if( _fmtfl & _MyBase::hex ) _Stream >> std::hex;
        if( _fmtfl & _MyBase::boolalpha ) _Stream >> std::boolalpha;
        if( _fmtfl & _MyBase::fixed ) _Stream >> std::fixed;
        if( _fmtfl & _MyBase::internal ) _Stream >> std::internal;
        if( _fmtfl & _MyBase::left ) _Stream >> std::left;
        if( _fmtfl & _MyBase::right ) _Stream >> std::right;
        if( _fmtfl & _MyBase::scientific ) _Stream >> std::scientific;
        if( _fmtfl & _MyBase::showbase ) _Stream >> std::showbase;
        if( _fmtfl & _MyBase::showpoint ) _Stream >> std::showpoint;
        if( _fmtfl & _MyBase::showpos ) _Stream >> std::showpos;
        if( _fmtfl & _MyBase::skipws ) _Stream >> std::skipws;
        if( _fmtfl & _MyBase::unitbuf ) _Stream >> std::unitbuf;
        if( _fmtfl & _MyBase::uppercase ) _Stream >> std::uppercase;
        return _Stream;
        }

    };

using socketstream = basic_socketstream<char>;
using wsocketstream = basic_socketstream<wchar_t>;


template<typename _Elem, typename _Traits>
inline void swap( basic_socketstream<_Elem, _Traits>& _Left, basic_socketstream<_Elem, _Traits>& _Right ) noexcept
    {	// exchange values stored at _Left and _Right
    _Left.swap( _Right );
    }


}// libsock

#endif// RC_INVOKED
#endif// __libsock_h_
