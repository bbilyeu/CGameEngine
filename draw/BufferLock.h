#ifndef BUFFERLOCK_H
#define BUFFERLOCK_H
#include "common/types.h"
#include <vector>

/**
 * \file BufferLock.h
 * \brief Header-only file to handle OpenGL direct state access (DSA) and memory fencing
 */

GLuint64 oneSecondInNanoSeconds = 1000000000;

namespace CGameEngine
{
	/**
	* BufferRange object to store start and length of a locked buffer
	*/
	struct BufferRange
	{
		size_t offsetStart; /** start of the buffer range */
		size_t length; /** size of the locked  buffer range */

		/**
		 * determines if ranges overlap
		 *
		 * @param other another buffer range
		 */
		bool overlaps(const BufferRange& other)
		{
			return ( offsetStart < (other.offsetStart + other.length)
					 && other.offsetStart < (offsetStart + length) );
		}
	};

	/**
	 * Object to track a buffer being locked
	 */
	struct BufferLock
	{
        BufferRange range; /** Range object which is locked */
        GLsync syncID; /** ID provided by OpenGL during a lock */
	};

	/**
	 * Object to handle the locking and cycling of ranges
	 */
    class BufferLockManager
    {
		public:
            BufferLockManager(bool useCPUforUpdates = true) : m_cpuUpdating(useCPUforUpdates) { }
            ~BufferLockManager() { for(auto it = m_locks.begin(); it != m_locks.end(); ++it) { cleanup(&*it); } m_locks.clear();  }

			/**
			 * determine if the locked range is writable
			 *
			 * @param lockOffsetStart beginning of the BufferRange
			 * @param lockLength size of the BufferRange (BufferRange.length)
			 */
            void waitForLockedRange(size_t lockOffsetStart, size_t lockLength)
            {
				BufferRange br = { lockOffsetStart, lockLength };
				std::vector<BufferLock> swapVec = std::vector<BufferLock>();
				for(auto it = m_locks.begin(); it != m_locks.end(); ++it)
				{
					// if an overlap occurs, wait until this has resolved, or data trampling will occur
                    if(br.overlaps(it->range))
                    {
                        try { wait(it->syncID); }
                        catch(std::exception& e) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "BufferLockManager::waitForLockedRange()", "Failed during wait(). Message: {}", e.what()); }
                        cleanup(&*it);
                    }
                    else { swapVec.push_back(*it); }
				}
				m_locks.swap(swapVec);
            }

            /**
			 * fire off the glFenceSync, creating a BufferRange and BufferLock in the process
			 *
			 * @param lockOffsetStart beginning of the BufferRange
			 * @param lockLength size of the BufferRange (BufferRange.length)
			 */
            void lockRange(size_t lockOffsetStart, size_t lockLength)
            {
                BufferRange br = { lockOffsetStart, lockLength };
                GLsync syncID = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
                BufferLock bl = { br, syncID };
                m_locks.push_back(bl);
            }

			/**
			 * glFenceSync (locK) off an existing range object
			*/
            void lockRange(GLsync& gsync)
            {
				if(gsync) { glDeleteSync(gsync); }
				gsync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            }

		private:
			bool m_cpuUpdating = true;
			std::vector<BufferLock> m_locks; /** store the BufferLocks in a vector */

			/**
			 * used to determine who is causing the hold up. cpu or video device
			 *
			 * @param syncID passed sync ID to check against
			*/
			void wait(GLsync& syncID)
			{
				if(m_cpuUpdating)
				{
					GLbitfield waitFlags = 0;
					GLuint64 waitDuration = 0;
					while(true)
					{
						GLenum waitRetVal = glClientWaitSync(syncID, waitFlags, waitDuration);

						if(waitRetVal == GL_ALREADY_SIGNALED || waitRetVal == GL_CONDITION_SATISFIED) { return; } // success
						else if(waitRetVal == GL_WAIT_FAILED) { Logger::getInstance().Log(Logs::FATAL, Logs::DataValidation, "BufferLockManager::wait()", "glClientWaitSync returned GL_WAIT_FAILED."); }  // fatal error

						// timeout hit, wait a disappointingly long time
						waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
						waitDuration = oneSecondInNanoSeconds;
					}
				}
				// wait on gpu to tell us it is ready
				else
				{
                    try { glWaitSync(syncID, 0, GL_TIMEOUT_IGNORED); }
                    catch(std::exception& e) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "BufferLockManager::wait()", "Failed with message: {}", e.what()); }
				}
			}

			/**
			 * safely delete a glSync object
			 */
			void cleanup(BufferLock* bl)
			{
				glDeleteSync(bl->syncID);
			}
    };
}

#endif // BUFFERLOCK_H
