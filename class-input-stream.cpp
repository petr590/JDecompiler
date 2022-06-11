#ifndef JDECOMPILER_CLASS_BINARY_INPUT_STREAM_CPP
#define JDECOMPILER_CLASS_BINARY_INPUT_STREAM_CPP

namespace jdecompiler {
	struct ClassInputStream final: BinaryInputStream {
		private:
			BinaryInputStream& instream;

		public:
			const string fileName;

			ClassInputStream(BinaryInputStream& instream, const string& fileName):
					instream(instream), fileName(fileName) {}

			ClassInputStream(FileBinaryInputStream& instream):
					instream(instream), fileName(instream.path) {}

			inline virtual const streampos& getPos() const override {
				return instream.getPos();
			}

			inline virtual void setPosTo(const streampos& pos) {
				instream.setPosTo(pos);
			}

			inline virtual size_t available() const override {
				return instream.available();
			}

			inline virtual int8_t readByte() override {
				return instream.readByte();
			}

			inline virtual uint8_t readUByte() override {
				return instream.readUByte();
			}

			inline virtual int16_t readShort() override {
				return instream.readShort();
			}

			inline virtual uint16_t readUShort() override {
				return instream.readUShort();
			}

			inline virtual int32_t readInt() override {
				return instream.readInt();
			}

			inline virtual uint32_t readUInt() override {
				return instream.readUInt();
			}

			inline virtual int64_t readLong() override {
				return instream.readLong();
			}

			inline virtual uint64_t readULong() override {
				return instream.readULong();
			}


			inline virtual float readFloat() override {
				return instream.readFloat();
			}

			inline virtual double readDouble() override {
				return instream.readDouble();
			}


			inline virtual const uint8_t* readBytes(uint32_t size) override {
				return instream.readBytes(size);
			}

			inline virtual const char* readString(uint32_t size) override {
				return instream.readString(size);
			}


			inline virtual void close() override {
				instream.close();
			}


			inline virtual ~ClassInputStream() {
				delete &instream;
			}
	};
}

#endif
