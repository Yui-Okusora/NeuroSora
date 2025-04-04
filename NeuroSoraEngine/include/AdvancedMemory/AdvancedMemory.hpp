#pragma once

#define NeuroSoraCore_AdvancedMemory

#include "NeuroSoraCore.hpp"

#include "MemoryManager/MemoryManager.hpp"

namespace NeuroSoraCore
{

	class ViewOfAdvancedMemory;


	class AdvancedMemory
	{
	public:
		friend class MemoryManager;

		ViewOfAdvancedMemory&	load(size_t offset, size_t size); // offset and size in bytes
		void					unload(ViewOfAdvancedMemory& view);
		void					unloadAll();
		void					resize(const size_t& fileSize); // in bytes

		void*					getViewPtr(const ViewOfAdvancedMemory& view) const;

		bool					isValid() const;
		const DWORD&			getFileSize() const;

		inline unsigned long	getID() const;
		inline void				createMapObj();


		template<typename T>
		T& refAt(const size_t& index, const ViewOfAdvancedMemory& view) // recommended using
		{
			if (index >= (static_cast<size_t>(view.dwMapViewSize) - view.iViewDelta) / sizeof(T)) {
				throw std::out_of_range("Index out of range");
			}
			return *(reinterpret_cast<T*>(getViewPtr(view)) + index);
		}

		~AdvancedMemory();


		//--------- Thread-safe methods ----------

		ViewOfAdvancedMemory&	load_s(size_t offset, size_t size); // offset and size in bytes

		void					unload_s(ViewOfAdvancedMemory& view);
		void					unloadAll_s();
		void					resize_s(const size_t& fileSize); // in bytes
		void					createMapObj_s();

		void*					getViewPtr_s(const ViewOfAdvancedMemory& view) const;

		inline const DWORD&		getFileSize_s() const;
		inline unsigned long	getID_s() const;


		template<typename T>
		T& refAt_s(const size_t& index, const ViewOfAdvancedMemory& view)
		{
			std::unique_lock<std::shared_mutex> lock(*view.mutex);
			if (index >= (static_cast<size_t>(view.dwMapViewSize) - view.iViewDelta) / sizeof(T)) {
				lock.unlock();
				throw std::out_of_range("Index out of range");
			}
			lock.unlock();
			return *(reinterpret_cast<T*>(getViewPtr(view)) + index);
		}

		template<typename T>
		T readAt(const size_t& index, const ViewOfAdvancedMemory& view) const
		{
			std::shared_lock<std::shared_mutex> lock(*view.mutex);
			if (index >= (static_cast<size_t>(view.dwMapViewSize) - view.iViewDelta) / sizeof(T)) {
				lock.unlock();
				throw std::out_of_range("Index out of range");
			}
			lock.unlock();
			return *(reinterpret_cast<T*>(getViewPtr(view)) + index);
		}


	private:
		ViewOfAdvancedMemory&	_load(ViewOfAdvancedMemory& view, size_t offset, size_t size);

		void					closeAllPtr();
		void					closeAllPtr_s();
		void					reset();

		HANDLE& getFileHandle() { return m_hFile; }
		HANDLE& getMapHandle() { return m_hMapFile; }

		HANDLE m_hMapFile = NULL;     // handle for the file's memory-mapped region
		HANDLE m_hFile = NULL;        // the file handle

		DWORD dwFileSize = 0;		// temporary storage for file sizes  
		unsigned long m_fileID = 0;
		std::unordered_map<LPVOID, ViewOfAdvancedMemory> views;
		std::shared_ptr<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();
	};

	class ViewOfAdvancedMemory
	{
	public:
		ViewOfAdvancedMemory& operator=(const ViewOfAdvancedMemory& view)
		{
			lpMapAddress = view.lpMapAddress;
			dwMapViewSize = view.dwMapViewSize;
			iViewDelta = view.iViewDelta;
			_offset = view._offset;
			parent = view.parent;
			autoRelease = view.autoRelease;
			return *this;
		}


		friend class AdvancedMemory;
		LPVOID lpMapAddress = nullptr;		// first address of the mapped view
		DWORD dwMapViewSize = 0;		// the size of the view
		unsigned long iViewDelta = 0;
		unsigned long long _offset = 0;

		AdvancedMemory* parent = nullptr;
		long long autoRelease = 0;

		~ViewOfAdvancedMemory()
		{
			if (autoRelease != 0 && parent != nullptr)
			{
				parent->unload_s(*this);
			}
		}
	private:
		std::shared_ptr<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();
	};

}





