#ifndef STGEN_EVENTHANDLERS_H
#define STGEN_EVENTHANDLERS_H

#include <unistd.h>
#include <memory>
#include <fstream>

#include "spdlog/spdlog.h"
#include "zfstream.h"
#include "Sigil2/Backends.hpp"

#include "STShadowMemory.hpp"
#include "STEvent.hpp"

namespace STGen
{

/* parse any arguments */
void onParse(Args args);
void onExit();


////////////////////////////////////////////////////////////
// Local stats for each thread
// Required for CPI estimates in SynchroTrace
////////////////////////////////////////////////////////////
/* XXX MDL20161104
 * Stats implementation will cause lock contention in frontends that produce
 * frequent thread switching in event streams, such as DynamoRIO. */
struct PerThreadStats
{
    static constexpr TID INVL_TID = -1;
    TID curr_tid{INVL_TID};

    /* <iop, flop, read, write>) */
    enum Type {IOP = 0, FLOP, Read, Write, Instr};
    using Stats = std::tuple<StatCounter, StatCounter, StatCounter, StatCounter, StatCounter>;
    Stats curr_counts{0,0,0,0,0};

    static std::mutex stats_mutex;
    static std::unordered_map<TID, Stats> per_thread_counts;
    void sync();
    void setThread(TID tid);
};


////////////////////////////////////////////////////////////
// Shared between all threads
////////////////////////////////////////////////////////////
/* Protect access to pthread metadata generated for
 * certain sync events */
extern std::mutex sync_event_mutex;

/* Vector of spawner, address of spawnee thread_t
 * All addresses associated with the same spawner are in
 * the order they were inserted */
extern std::vector<std::pair<TID, Addr>> thread_spawns;

/* Each spawned thread's ID, in the order it first seen ('created') */
extern std::vector<TID> thread_creates;

/* Vector of barrier_t addr, participating threads.
 * Order matters for SynchroTraceSim */
extern std::vector<std::pair<Addr, std::set<TID>>> barrier_participants;


class EventHandlers : public BackendIface
{
    /* Shadow memory is shared amongst all instances,
     * with the implication that each instance may be
     * a separate thread's stream of events */
    static STShadowMemory shad_mem;

    /* Per-thread event count. Logged to SynchroTrace event trace.
     * Each derived SynchroTrace event tracks the same event id.  */
    std::unordered_map<TID, EID> event_ids;
    TID curr_thread_id;
    EID curr_event_id;

    /* Output directly to a *.gz stream to save space */
    /* Keep these ostreams open until deconstruction */
    std::vector<std::shared_ptr<gzofstream>> gz_streams;
    std::map<TID, std::shared_ptr<spdlog::logger>> loggers;
    std::shared_ptr<spdlog::logger> curr_logger;

    std::string filename(TID tid)
    {
        return output_directory + "/" + filebase + std::to_string(tid) + ".gz";
    }

  public:
    EventHandlers();
    EventHandlers(const EventHandlers &) = delete;
    EventHandlers &operator=(const EventHandlers &) = delete;
    ~EventHandlers();

    /* Compatibility with SynchroTraceSim parser */
    static const std::string filebase;
    static std::string output_directory;

    /* compression level of events */
    static unsigned int primitives_per_st_comp_ev;

    /* interface to Sigil2 */
    virtual void onSyncEv(const SglSyncEv &ev) override;
    virtual void onCompEv(const SglCompEv &ev) override;
    virtual void onMemEv(const SglMemEv &ev) override;
    virtual void onCxtEv(const SglCxtEv &ev) override;

    /* SynchroTraceGen makes use of 3 SynchroTrace events,
     * i.e. Computation, Communication, and Synchronization.
     *
     * One of each event is populated and flushed as Sigil
     * primitives are processed. Because there might be trillions
     * or more of SynchroTrace events, dynamic heap allocation of
     * consecutive SynchroTrace events is avoided */
    STCompEvent st_comp_ev;
    STCommEvent st_comm_ev;
    STSyncEvent st_sync_ev;
    PerThreadStats stats;

  private:
    void onLoad(const SglMemEv &ev_data);
    void onStore(const SglMemEv &ev_data);
    void setThread(TID tid);
    void initThreadLog(TID tid);
    void switchThreadLog(TID tid);
};

}; //end namespace STGen

#endif
