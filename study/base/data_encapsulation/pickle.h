#pragma once
#include <string>

namespace mctm
{
    class Pickle;

    // PickleIterator reads data from a Pickle. The Pickle object must remain valid
    // while the PickleIterator object is in use.
    class PickleIterator
    {
    public:
        PickleIterator() {}
        explicit PickleIterator(const Pickle& pickle);

        // Methods for reading the payload of the Pickle. To read from the start of
        // the Pickle, create a PickleIterator from a Pickle. If successful, these
        // methods return true. Otherwise, false is returned to indicate that the
        // result could not be extracted.
        bool ReadBool(bool* result);
        bool ReadInt(int* result);
        bool ReadLong(long* result);
        bool ReadUInt16(unsigned short* result);
        bool ReadUInt32(unsigned int* result);
        bool ReadInt64(__int64* result);
        bool ReadUInt64(unsigned __int64* result);
        bool ReadFloat(float* result);
        bool ReadString(std::string* result);
        bool ReadWString(std::wstring* result);
        bool ReadData(const char** data, int* length);
        bool ReadBytes(const char** data, int length);

        // Safer version of ReadInt() checks for the result not being negative.
        // Use it for reading the object sizes.
        bool ReadLength(int* result)
        {
            return ReadInt(result) && *result >= 0;
        }

        // Skips bytes in the read buffer and returns true if there are at least
        // num_bytes available. Otherwise, does nothing and returns false.
        bool SkipBytes(int num_bytes)
        {
            return !!GetReadPointerAndAdvance(num_bytes);
        }

    private:
        // Aligns 'i' by rounding it up to the next multiple of 'alignment'
        static size_t AlignInt(size_t i, int alignment)
        {
            return i + (alignment - (i % alignment)) % alignment;
        }

        // Read Type from Pickle.
        template <typename Type>
        inline bool ReadBuiltinType(Type* result);

        // Get read pointer for Type and advance read pointer.
        template<typename Type>
        inline const char* GetReadPointerAndAdvance();

        // Get read pointer for |num_bytes| and advance read pointer. This method
        // checks num_bytes for negativity and wrapping.
        const char* GetReadPointerAndAdvance(int num_bytes);

        // Get read pointer for (num_elements * size_element) bytes and advance read
        // pointer. This method checks for int overflow, negativity and wrapping.
        inline const char* GetReadPointerAndAdvance(int num_elements,
            size_t size_element);

        // Pointers to the Pickle data.
        const char* read_ptr_ = nullptr;
        const char* read_end_ptr_ = nullptr;
    };

    class Pickle
    {
    public:
        // Initialize a Pickle object using the default header size.
        Pickle();

        // Initialize a Pickle object with the specified header size in bytes, which
        // must be greater-than-or-equal-to sizeof(Pickle::Header).  The header size
        // will be rounded up to ensure that the header size is 32bit-aligned.
        explicit Pickle(int header_size);

        // Initializes a Pickle from a const block of data.  The data is not copied;
        // instead the data is merely referenced by this Pickle.  Only const methods
        // should be used on the Pickle when initialized this way.  The header
        // padding size is deduced from the data length.
        Pickle(const char* data, int data_len);

        // Initializes a Pickle as a deep copy of another Pickle.
        Pickle(const Pickle& other);

        // Note: There are no virtual methods in this class.  This destructor is
        // virtual as an element of defensive coding.  Other classes have derived from
        // this class, and there is a *chance* that they will cast into this base
        // class before destruction.  At least one such class does have a virtual
        // destructor, suggesting at least some need to call more derived destructors.
        virtual ~Pickle();

        // Performs a deep copy.
        Pickle& operator=(const Pickle& other);

        // Returns the size of the Pickle's data.
        size_t size() const { return header_size_ + header_->payload_size; }

        // Returns the data for this Pickle.
        const void* data() const { return header_; }

        // For compatibility, these older style read methods pass through to the
        // PickleIterator methods.
        // TODO(jbates) Remove these methods.
        bool ReadBool(PickleIterator* iter, bool* result) const
        {
            return iter->ReadBool(result);
        }
        bool ReadInt(PickleIterator* iter, int* result) const
        {
            return iter->ReadInt(result);
        }
        bool ReadLong(PickleIterator* iter, long* result) const
        {
            return iter->ReadLong(result);
        }
        bool ReadUInt16(PickleIterator* iter, unsigned short* result) const
        {
            return iter->ReadUInt16(result);
        }
        bool ReadUInt32(PickleIterator* iter, unsigned int* result) const
        {
            return iter->ReadUInt32(result);
        }
        bool ReadInt64(PickleIterator* iter, __int64* result) const
        {
            return iter->ReadInt64(result);
        }
        bool ReadUInt64(PickleIterator* iter, unsigned __int64* result) const
        {
            return iter->ReadUInt64(result);
        }
        bool ReadFloat(PickleIterator* iter, float* result) const
        {
            return iter->ReadFloat(result);
        }
        bool ReadString(PickleIterator* iter, std::string* result) const
        {
            return iter->ReadString(result);
        }
        bool ReadWString(PickleIterator* iter, std::wstring* result) const
        {
            return iter->ReadWString(result);
        }
        // A pointer to the data will be placed in *data, and the length will be
        // placed in *length. This buffer will be into the message's buffer so will
        // be scoped to the lifetime of the message (or until the message data is
        // mutated).
        bool ReadData(PickleIterator* iter, const char** data, int* length) const
        {
            return iter->ReadData(data, length);
        }
        // A pointer to the data will be placed in *data. The caller specifies the
        // number of bytes to read, and ReadBytes will validate this length. The
        // returned buffer will be into the message's buffer so will be scoped to the
        // lifetime of the message (or until the message data is mutated).
        bool ReadBytes(PickleIterator* iter, const char** data, int length) const
        {
            return iter->ReadBytes(data, length);
        }

        // Safer version of ReadInt() checks for the result not being negative.
        // Use it for reading the object sizes.
        bool ReadLength(PickleIterator* iter, int* result) const
        {
            return iter->ReadLength(result);
        }

        bool WriteBool(bool value)
        {
            return WriteInt(value ? 1 : 0);
        }
        bool WriteInt(int value)
        {
            return WriteBytes(&value, sizeof(value));
        }
        // WARNING: DO NOT USE THIS METHOD IF PICKLES ARE PERSISTED IN ANY WAY.
        // It will write whatever a "long" is on this architecture. On 32-bit
        // platforms, it is 32 bits. On 64-bit platforms, it is 64 bits. If persisted
        // pickles are still around after upgrading to 64-bit, or if they are copied
        // between dissimilar systems, YOUR PICKLES WILL HAVE GONE BAD.
        bool WriteLongUsingDangerousNonPortableLessPersistableForm(long value)
        {
            return WriteBytes(&value, sizeof(value));
        }
        bool WriteUInt16(unsigned short value)
        {
            return WriteBytes(&value, sizeof(value));
        }
        bool WriteUInt32(unsigned int value)
        {
            return WriteBytes(&value, sizeof(value));
        }
        bool WriteInt64(__int64 value)
        {
            return WriteBytes(&value, sizeof(value));
        }
        bool WriteUInt64(unsigned __int64 value)
        {
            return WriteBytes(&value, sizeof(value));
        }
        bool WriteFloat(float value)
        {
            return WriteBytes(&value, sizeof(value));
        }
        bool WriteString(const std::string& value);
        bool WriteWString(const std::wstring& value);
        // "Data" is a blob with a length. When you read it out you will be given the
        // length. See also WriteBytes.
        bool WriteData(const char* data, int length);
        // "Bytes" is a blob with no length. The caller must specify the lenght both
        // when reading and writing. It is normally used to serialize PoD types of a
        // known size. See also WriteData.
        bool WriteBytes(const void* data, int data_len);

        // Same as WriteData, but allows the caller to write directly into the
        // Pickle. This saves a copy in cases where the data is not already
        // available in a buffer. The caller should take care to not write more
        // than the length it declares it will. Use ReadData to get the data.
        // Returns NULL on failure.
        //
        // The returned pointer will only be valid until the next write operation
        // on this Pickle.
        char* BeginWriteData(int length);

        // For Pickles which contain variable length buffers (e.g. those created
        // with BeginWriteData), the Pickle can
        // be 'trimmed' if the amount of data required is less than originally
        // requested.  For example, you may have created a buffer with 10K of data,
        // but decided to only fill 10 bytes of that data.  Use this function
        // to trim the buffer so that we don't send 9990 bytes of unused data.
        // You cannot increase the size of the variable buffer; only shrink it.
        // This function assumes that the length of the variable buffer has
        // not been changed.
        void TrimWriteData(int length);

        // Payload follows after allocation of Header (header size is customizable).
        struct Header
        {
            unsigned int payload_size = 0;  // Specifies the size of the payload.
        };

        template <class T>
        T* headerT()
        {
            //DCHECK_EQ(header_size_, sizeof(T));
            return static_cast<T*>(header_);
        }

        template <class T>
        const T* headerT() const
        {
            //DCHECK_EQ(header_size_, sizeof(T));
            return static_cast<const T*>(header_);
        }

        // The payload is the pickle data immediately following the header.
        size_t payload_size() const { return header_->payload_size; }

        const char* payload() const
        {
            return reinterpret_cast<const char*>(header_) + header_size_;
        }

        // Returns the address of the byte immediately following the currently valid
        // header + payload.
        const char* end_of_payload() const
        {
            // This object may be invalid.
            return header_ ? payload() + payload_size() : NULL;
        }

    protected:
        char* mutable_payload()
        {
            return reinterpret_cast<char*>(header_) + header_size_;
        }

        size_t capacity() const
        {
            return capacity_;
        }

        // Resizes the buffer for use when writing the specified amount of data. The
        // location that the data should be written at is returned, or NULL if there
        // was an error. Call EndWrite with the returned offset and the given length
        // to pad out for the next write.
        char* BeginWrite(size_t length);

        // Completes the write operation by padding the data with NULL bytes until it
        // is padded. Should be paired with BeginWrite, but it does not necessarily
        // have to be called after the data is written.
        void EndWrite(char* dest, int length);

        // Resize the capacity, note that the input value should include the size of
        // the header: new_capacity = sizeof(Header) + desired_payload_capacity.
        // A realloc() failure will cause a Resize failure... and caller should check
        // the return result for true (i.e., successful resizing).
        bool Resize(size_t new_capacity);

        // Aligns 'i' by rounding it up to the next multiple of 'alignment'
        static size_t AlignInt(size_t i, int alignment)
        {
            return i + (alignment - (i % alignment)) % alignment;
        }

        // Find the end of the pickled data that starts at range_start.  Returns NULL
        // if the entire Pickle is not found in the given data range.
        static const char* FindNext(size_t header_size,
            const char* range_start,
            const char* range_end);

        // The allocation granularity of the payload.
        static const int kPayloadUnit;

    private:
        friend class PickleIterator;

        Header* header_ = nullptr;
        size_t header_size_ = 0;  // Supports extra data between header and payload.
        // Allocation size of payload (or -1 if allocation is const).
        size_t capacity_ = 0;
        size_t variable_buffer_offset_ = 0;  // IF non-zero, then offset to a buffer.
    };


}