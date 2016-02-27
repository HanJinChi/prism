#ifndef STGEN_EVENT_H
#define STGEN_EVENT_H

#include <vector>
#include <set>
#include <unordered_map>
#include <memory>
#include "BlockPoolAllocator.hpp"
#include "Sigil2/Primitive.h"
#include "spdlog.h"
#include "ShadowMemory.hpp"

/*******************************************************************************
 * SynchroTrace Events
 *
 * Defines an application event trace for later replay
 * in the SynchroTraceSim CMP simulator.
 *
 * Please see Section 2.B in the SynchroTrace paper for further clarification.
 ******************************************************************************/

namespace STGen
{
using std::shared_ptr;
using std::pair;
using std::tuple;
using std::vector;
using std::multiset;

/**
 * A SynchroTrace Compute Event.
 *
 * SynchroTrace compute events comprise of one or more Sigil primitive events.
 * I.e., a ST compute event can be any number of local thread load, store,
 * and compute primitives. However in practice, a 'compute' event is normally 
 * cut off at ~100 event primitives.
 * 
 * A SynchroTrace compute event is only valid for a given thread. 
 * Usage is to fill up the event, then flush it to storage at a thread swap,
 * a communication edge between threads, or at an arbitrary number 
 * of iops/flops/reads/writes.
 */
struct STCompEvent 
{
	/* Helper class to track unique ranges of addresses */
	struct AddrSet
	{
		using AddrRange = pair<Addr,Addr>;
	private:
		multiset<AddrRange, std::less<AddrRange>, BlockPoolAllocator<AddrRange>> ms;
	public:
		/* A range of addresses is specified by the pair.
		 * This call inserts that range and merges existing ranges
		 * in order to keep the set of addresses unique */
		void insert(const AddrRange &range);
		void clear();
		const decltype(ms)& get(){ return ms; }
	};

	UInt iop_cnt;
	UInt flop_cnt;	

	/* Stores and Loads originating from the current thread
	 * I.e. non-edge mem events. These are the count for 'events',
	 * not a count for bytes stored/loaded */
	UInt store_cnt;
	UInt load_cnt;

	/* Holds a range for the addresses touched by local stores/loads */
	AddrSet stores_unique; 
	AddrSet loads_unique;

	UInt total_events;
	bool is_empty;

	TId &thread_id;
	EId &event_id;
	const shared_ptr<spdlog::logger> &logger;

public:
	STCompEvent(TId &tid, EId &eid, const shared_ptr<spdlog::logger> &logger);
	void flush();
	void updateWrites(SglMemEv ev);
	void updateWrites(Addr begin, Addr size);
	void updateReads(SglMemEv ev);
	void updateReads(Addr begin, Addr size);
	void incIOP();
	void incFLOP();
	
private:
	void reset();
};

/**
 * A SynchroTrace Communication Event.
 *
 * SynchroTrace communication events comprise of communication edges.
 * I.e., a ST comm event is typically generated from a read to data
 * that was originally created by other threads. Any subsequent reads
 * to that data by the same thread is not considered a communication 
 * edge. Another thread that writes to the same address resets this process.
 */
struct STCommEvent
{
	typedef vector<tuple<TId, EId, Addr, Addr>> LoadEdges;
	/**< vector of: 
	 *     producer thread id, 
	 *     producer event id, 
	 *     addr range
	 * for reads to data written by another thread
	 */

	LoadEdges comms;	
	bool is_empty;

	TId &thread_id;
	EId &event_id;
	const shared_ptr<spdlog::logger> &logger;

public:
	STCommEvent(TId &tid, EId &eid, const shared_ptr<spdlog::logger> &logger);
	void flush();

	/** 
	 * Adds communication edges originated from a single load/read primitive.
	 * Use this function when reading data that was written by a different thread.
	 *
	 * Expected to by called byte-by-byte. 
	 * That is, successive calls to this function imply a continuity 
	 * between addresses. If the first call specifies address 0x0000,
	 * and the next call specifies address 0x0008, then it implies
	 * addresses 0x0000-0x0008 were all read, instead of non-consecutively read.
	 *
	 * Use STEvent::flush() between different read primitives.
	 */
	void addEdge(TId writer, EId writer_event, Addr addr);

private:
	void reset();
};

/**
 * A SynchroTrace Synchronization Event.
 *
 * Synchronization events mark thread spawns, joins, barriers, locks, et al.
 * Expected use is to flush immediately; these events do not aggregate like 
 * SynchroTrace Compute or Communication events.
 *
 * Synchronizatin events are expected to be immediately logged.
 */
class STSyncEvent
{
	UChar type = 0;
	Addr sync_addr = 0;

	TId &thread_id;
	EId &event_id;
	const shared_ptr<spdlog::logger> &logger;

public:
	STSyncEvent(TId &tid, EId &eid, const shared_ptr<spdlog::logger> &logger);

	/* only behavior is immediate flush */
	void flush(UChar type, Addr sync_addr);
};

}; //end namespace STGen

#endif
