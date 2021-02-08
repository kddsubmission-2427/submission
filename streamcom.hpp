#ifndef HDRFPP__STREAMCOM_HPP_
#define HDRFPP__STREAMCOM_HPP_

#include <unordered_map>
#include <fstream>

#include "globals.hpp"
#include "twophasepartitioner.hpp"

DECLARE_string(streamcom_type);

class Streamcom
{
private:
    std::vector<uint32_t> communities; // index is vertex id, community of a vertex
    std::vector<uint64_t> volumes; // index is community id, volume of a community
    uint32_t next_community_id;
    Globals &globals;

public:
    explicit Streamcom(const Globals& GLOBALS);
    std::vector<uint32_t> find_communities();
    std::vector<uint64_t> get_volumes();
    void do_streamcom(std::vector<edge_t> &edges);
    typedef std::vector<std::pair<uint64_t, uint32_t>> sort_communities();
};

#endif //HDRFPP__STREAMCOM_HPP_
