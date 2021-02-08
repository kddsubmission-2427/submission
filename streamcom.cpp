#include "streamcom.hpp"

DECLARE_string(communities_file);
DECLARE_int32(str_iters);

Streamcom::Streamcom(const Globals &GLOBALS) : globals(const_cast<Globals &>(GLOBALS))
{
    communities.resize(GLOBALS.NUM_VERTICES, 0);
    volumes.resize(GLOBALS.NUM_VERTICES + 1, 0);
    next_community_id = 1;
}

void find_com_forwarder(void* object, std::vector<edge_t> edges)
{
    static_cast<Streamcom*>(object)->do_streamcom(edges);
}

std::vector<uint32_t> Streamcom::find_communities()
{
    // Decreasing community volume in the first run.
    // This makes to smaller communities after the first communities so that they would be avaiable
    // for extending in the second clustering
    
    switch (FLAGS_str_iters)
    {
        LOG(INFO) << "Community detection... [will be processed " << FLAGS_str_iters << " times]";
        case 1:
            globals.read_and_do(find_com_forwarder, this, "communities");
            break;
        default:
            globals.MAX_COM_VOLUME /= 2;
            globals.read_and_do(find_com_forwarder, this, "communities");
            globals.MAX_COM_VOLUME *= 2;
            globals.read_and_do(find_com_forwarder, this, "communities");

            for (int i = 3; i <= FLAGS_str_iters; i++)
            {
               globals.read_and_do(find_com_forwarder, this, "communities"); 
            }
            break;
    }
    return communities;
}

void Streamcom::do_streamcom(std::vector<edge_t> &edges)
{
    for (auto& edge : edges)
    {
        auto u = edge.first;
        auto v = edge.second;

        auto& com_u = communities[u];
        auto& com_v = communities[v];
        if(com_u == 0)
        {
            com_u = next_community_id;
            volumes[com_u] += globals.DEGREES[u];
            ++next_community_id;
        }
        if(com_v == 0)
        {
            com_v = next_community_id;
            volumes[com_v] += globals.DEGREES[v];
            ++next_community_id;
        }

        auto& vol_u = volumes[com_u];
        auto& vol_v = volumes[com_v];
        if((vol_u <= globals.MAX_COM_VOLUME) && (vol_v <= globals.MAX_COM_VOLUME))
        {
            if(vol_u <= vol_v && vol_v + globals.DEGREES[u] <= globals.MAX_COM_VOLUME)
            {
                vol_u -= globals.DEGREES[u];
                vol_v += globals.DEGREES[u];
                communities[u] = communities[v];
            }
            if (vol_v < vol_u && vol_u + globals.DEGREES[v] <= globals.MAX_COM_VOLUME)
            {
                vol_v -= globals.DEGREES[v];
                vol_u += globals.DEGREES[v];
                communities[v] = communities[u];
            }
        }
    }
}

std::vector<uint64_t> Streamcom::get_volumes()
{
    return volumes;
}


