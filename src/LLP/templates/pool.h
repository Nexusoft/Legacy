/*__________________________________________________________________________________________
 
			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2017] ++
			
			(c) Copyright The Nexus Developers 2014 - 2017
			
			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.
			
			"fides in stellis, virtus in numeris" - Faith in the Stars, Power in Numbers
  
____________________________________________________________________________________________*/

#ifndef NEXUS_LLP_TEMPLATES_POOL_H
#define NEXUS_LLP_TEMPLATES_POOL_H

#include "../../Core/include/unifiedtime.h"
#include "../../Util/include/mutex.h"

namespace LLP
{	

	/* Holding Object for Memory Maps. */
	template<typename ObjectType> class CHoldingObject
	{
	public:
		uint64        Timestamp;
		unsigned char     State;
		ObjectType       Object;
		
		CHoldingObject() {}
		CHoldingObject(uint64 TimestampIn, unsigned char StateIn, ObjectType ObjectIn) : Timestamp(TimestampIn), State(StateIn), Object(ObjectIn) {}
	};
	
	
	/** Holding Pool:
	 * 
	 * This class is responsible for holding data that is partially processed.
	 * It is also uselef for data that needs to be relayed from cache once recieved.
	 * 
	 * It must adhere to the processing expiration time.
	 * 
	 * A. It can pass data on the relay layers if required.
	 * B. It can process data locked as orphans
	 * 
	 */
	template<typename IndexType, typename ObjectType, typename HoldingType = CHoldingObject<ObjectType> > class CHoldingPool
	{
			
		/* Map of the current holding data. */
		std::map<IndexType, CHoldingObject<ObjectType> > mapObjects;
		
		/* The Expiration Time of Holding Data. */
		unsigned int nExpirationTime;
		
	protected:
		
		Mutex_t MUTEX;
		
	public:
	
		/** State level messages to hold information about holding data. */
		enum
		{
			//Validation States
			VERIFIED   = 0,
			ACCEPTED   = 1,
			INDEXED    = 2,
			ORPHANED   = 3,
			REJECTED   = 4,
			INVALID    = 5,
			INVENTORY  = 6,
			
			
			//Unverified States
			UNVERIFIED = 128,
			NOTFOUND   = 129
			
			//All states on 130 - 255 for custom states with child classes
		};
		
		/** Base Constructor. **/
		CHoldingPool() : mapObjects(), nExpirationTime(0) {}
		
		
		/** Expiration Constructor
		 * 
		 * @param[in] nExpirationTimeIn The time in seconds for objects to expire in the pool.
		 * 
		 */
		CHoldingPool(unsigned int nExpirationTimeIn) : mapObjects(), nExpirationTime(nExpirationTimeIn) {}
		
		
		/** Check for Data by Index.
		 * 
		 * @param[in] Index Template arguement to check by supplied index
		 * 
		 * @return Boolean expressino whether pool contains data by index
		 * 
		 */
		bool Has(IndexType Index) 
		{ 
			LOCK(MUTEX); 
			
			return (mapObjects.find(Index) != mapObjects.end()); 
		}
		
		
		/** Get the Data by Index
		 * 
		 * @param[in] Index Template argument to get by supplied index
		 * @param[out] Object Reference variable to return object if found
		 * 
		 * @return True if object was found, false if none found by index.
		 * 
		 */
		bool Get(IndexType Index, ObjectType& Object)
		{
			LOCK(MUTEX);
			
			if(!Has(Index))
				return false;
			
			Object = mapObjects[Index].Object;
			
			return true;
		}
		
		
		/** Get the Data by State.
		 * 
		 * @param[in] State The state that is being searched for 
		 * @param[out] vObjects The list of objects being sent out
		 * 
		 * @return Returns true if any states matched, false if none matched
		 * 
		 */
		bool Get(unsigned char State, std::vector<ObjectType>& vObjects)
		{
			LOCK(MUTEX);
			
			for(auto i : mapObjects)
				if(i.second.State == State)
					vObjects.push_back(i.second.Object);
				
			return (vObjects.size() > 0);
		}
		
		
		/** Get the Indexes in Pool
		 * 
		 * @param[out] vIndexes A list of Indexes in the pool
		 * 
		 * @return Returns true if there are indexes, false if none found.
		 * 
		 */
		bool GetIndexes(std::vector<IndexType>& vIndexes)
		{
			LOCK(MUTEX);
			
			for(auto i : mapObjects)
				vIndexes.push_back(i.first);
			
			return (vIndexes.size() > 0);
		}
		
		
		/** Get the Indexs in Pool by State
		 * 
		 * @param[in] State The state char to filter results by
		 * @param[out] vIndexes The return vector with the results
		 * 
		 * @return Returns true if indexes fit criteria, false if none found
		 * 
		 */
		bool GetIndexes(unsigned char State, std::vector<IndexType>& vIndexes)
		{
			LOCK(MUTEX);
			
			for(auto i : mapObjects)
				if(i.second.State == State)
					vIndexes.push_back(i.first);
				
			return (vIndexes.size() > 0);
		}
		
		
		/** Add data to the pool
		 * 
		 * Default state is UNVERIFIED
		 * 
		 * @param[in] Index Template argument to add selected index
		 * @param[in] Object Template argument for the object to be added.
		 * 
		 */
		bool Add(IndexType Index, ObjectType Object, char State = UNVERIFIED, uint64 nTimestamp = Core::UnifiedTimestamp())
		{
			if(Has(Index))
				return false;
			
			LOCK(MUTEX);
			
			HoldingType HoldingObject(nTimestamp, State, Object);
			mapObjects[Index] = HoldingObject;
			
			return true;
		}
		
		
		/** Update data in the Pool
		 * 
		 * Default state is UNVERIFIED
		 * 
		 * @param[in] Index Template argument to add selected index
		 * @param[in] Object Template argument for the object to be added.
		 * 
		 */
		bool Update(IndexType Index, ObjectType Object, char State = UNVERIFIED, uint64 nTimestamp = Core::UnifiedTimestamp())
		{
			if(!Has(Index))
				return false;
			
			LOCK(MUTEX);
			
			mapObjects[Index].Object    = Object;
			mapObjects[Index].State     = State;
			mapObjects[Index].Timestamp = nTimestamp;
			
			return true;
		}

		
		/** Set the State of specific Object
		 * 
		 * @param[in] Index Template argument to add selected index
		 * @param[in] State The selected state to add (Default is UNVERIFIED)
		 * 
		 */
		void SetState(IndexType Index, unsigned char State = UNVERIFIED)
		{ 
			LOCK(MUTEX);
			
			mapObjects[Index].State = State; 
		}
		
		
		/** Get the State of Specific Object
		 * 
		 * @param[in] Index Template argument to add selected index
		 * 
		 * @return Object state (NOTFOUND returned if does not exist)
		 *
		 */
		unsigned char State(IndexType Index)
		{
			if(!Has(Index))
				return NOTFOUND;
			
			LOCK(MUTEX);
			
			return mapObjects[Index].State; 
		}
		
		
		/** Force Remove Object by Index
		 * 
		 * @param[in] Index Template argument to determine location
		 * 
		 * @return True on successful removal, false if it fails
		 * 
		 */
		bool Remove(IndexType Index)
		{
			if(!Has(Index))
				return false;
			
			LOCK(MUTEX);
			
			mapObjects.erase(Index);
			
			return true;
		}
		
		
		/** Check if an object is Expired
		 * 
		 * @param[in] Index Template argument to determine location
		 * 
		 * @return True if object has expired, false if it is active
		 * 
		 */
		bool Expired(IndexType Index)
		{
			if(!Has(Index))
				return true;
			
			LOCK(MUTEX);
			
			if(mapObjects[Index].Timestamp + nExpirationTime < Core::UnifiedTimestamp())
				return true;
			
			return false;
		}
		
		
		/** Clean up data that is expired to keep memory use low
		 * 
		 * @return The number of elements that were removed in cleaning process.
		 * 
		 */
		int Clean()
		{
			LOCK(MUTEX);
			
			std::vector<IndexType> vClean;
			for(auto i : mapObjects)
				if(Expired(i.first))
					vClean.push_back(i.first);
			
			for(auto i : vClean)
				Remove(i);
			
			return vClean.size();
		}
		
		/** Count or the number of elements in the memory pool
		 * 
		 * @return returns the total size of the pool.
		 * 
		 */
		int Count()
		{
			LOCK(MUTEX);
			
			return mapObjects.size();
		}
		
		
		/** Count or the number of elements in the pool
		 * 
		 * @param[in] State The state to filter the results with
		 * 
		 * @return Returns the total number of elements in the pool based on the state
		 * 
		 */
		int Count(unsigned char State)
		{
			LOCK(MUTEX);
			
			int nCount = 0;
			for(auto i : mapObjects)
				if(i.second.State == State)
					nCount++;
				
			return nCount;
		}
	};
}

#endif
