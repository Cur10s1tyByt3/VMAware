#include "../techniques.hpp"
#include "../../types.hpp"
#include "../../modules/util.hpp"
#include "../../modules/cpu.hpp"
#include "../../modules/core.hpp"
#include "../../brands.hpp"
#include "../../includes.hpp"

/**
 * @brief Check if mac address starts with certain VM designated values
 * @category Linux and Windows
 * @implements VM::MAC
 */
[[nodiscard]] bool techniques::mac_address_check() {
    // C-style array on purpose
    u8 mac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#if (LINUX)
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    i32 success = 0;

    i32 sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (sock == -1) {
        return false;
    };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;

    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        return false;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        std::strcpy(ifr.ifr_name, it->ifr_name);

        if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0) {
            return false;
        }

        if (!(ifr.ifr_flags & IFF_LOOPBACK)) {
            if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                success = 1;
                break;
            }
        }
    }

    if (success) {
        std::memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    } else {
        debug("MAC: ", "not successful");
    }
#elif (WINDOWS)
    DWORD dwBufLen = 0;
    if (GetAdaptersInfo(nullptr, &dwBufLen) != ERROR_BUFFER_OVERFLOW) {
        return false;
    }

    PIP_ADAPTER_INFO AdapterInfo = (PIP_ADAPTER_INFO)std::malloc(dwBufLen);
    if (AdapterInfo == nullptr) {
        return false;
    }

    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
        std::memcpy(mac, AdapterInfo->Address, sizeof(mac));
    }
    std::free(AdapterInfo);
#else
    return false;
#endif

#ifdef __VMAWARE_DEBUG__
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << std::hex
        << static_cast<i32>(mac[0]) << ":"
        << static_cast<i32>(mac[1]) << ":"
        << static_cast<i32>(mac[2]) << ":XX:XX:XX";
    /* removed for privacy reasons, only the first 3 bytes are needed
        << static_cast<i32>(mac[3]) << ":"  
        << static_cast<i32>(mac[4]) << ":"
        << static_cast<i32>(mac[5]);
    */
    debug("MAC: ", ss.str());
#endif
    auto compare = [mac](u8 mac1, u8 mac2, u8 mac3) noexcept -> bool {
        return (mac[0] == mac1 && mac[1] == mac2 && mac[2] == mac3);
        };

    // Check for known virtualization MAC address prefixes
    if (compare(0x08, 0x00, 0x27))
        return core::add(brands::VBOX);

    if (compare(0x00, 0x0C, 0x29) ||
        compare(0x00, 0x1C, 0x14) ||
        compare(0x00, 0x50, 0x56) ||
        compare(0x00, 0x05, 0x69))
    {
        return core::add(brands::VMWARE);
    }

    if (compare(0x00, 0x16, 0xE3))
        return core::add(brands::XEN);

    if (compare(0x00, 0x1C, 0x42))
        return core::add(brands::PARALLELS);

    /*
        see https://github.com/kernelwernel/VMAware/issues/105
        if (compare(0x0A, 0x00, 0x27)) {
            return core::add(brands::HYBRID);
        }
    */

    return false;
}