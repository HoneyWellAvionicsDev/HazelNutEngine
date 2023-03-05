#pragma once

namespace Jbonk
{
	enum class ShaderDataType : uint8_t
	{
		None = 0, 
		Float, Float2, Float3, Float4, 
		Mat3, Mat4, 
		Int, Int2, Int3, Int4, 
		Bool, Struct
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:             return 4;
			case ShaderDataType::Float2:            return 8;
			case ShaderDataType::Float3:            return 12;
			case ShaderDataType::Float4:            return 16;
			case ShaderDataType::Mat3:              return 4 * 3 * 3;
			case ShaderDataType::Mat4:              return 4 * 4 * 4;
			case ShaderDataType::Int:               return 4;
			case ShaderDataType::Int2:              return 8;
			case ShaderDataType::Int3:              return 12;
			case ShaderDataType::Int4:              return 16;
			case ShaderDataType::Bool:              return 1;
		}
		JB_CORE_ASSERT(false, "Unknown ShaderDataType!")
		return 0;
	}

	struct BufferElements
	{
		std::string Name;
		ShaderDataType Type;
		size_t Offset;
		uint32_t Size;
		bool Normalized;

		BufferElements() = default;
		 
		BufferElements(ShaderDataType type, const std::string& name, bool normalzied = false)
			: Type(type), Name(name), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalzied)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:             return 1;
				case ShaderDataType::Float2:            return 2;
				case ShaderDataType::Float3:            return 3;
				case ShaderDataType::Float4:            return 4;
				case ShaderDataType::Int:               return 1;
				case ShaderDataType::Int2:              return 2;
				case ShaderDataType::Int3:              return 3;
				case ShaderDataType::Int4:              return 4;
				case ShaderDataType::Mat3:              return 3;
				case ShaderDataType::Mat4:              return 4;
				case ShaderDataType::Bool:              return 1;
			}
			JB_CORE_ASSERT(false, "Unknown ShaderDataType!")
			return 0;
		}

	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(std::initializer_list<BufferElements> elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}
		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElements>& GetElements() const { return m_Elements; } 
		
		std::vector<BufferElements>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElements>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElements>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElements>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for (auto& elements : m_Elements)
			{
				elements.Offset = offset;
				offset += elements.Size;
				m_Stride += elements.Size;
			}

		}

		std::vector<BufferElements> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;


		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
	};

	//only 32bit index buffers
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static  Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);  //possibly make this 16 bit for optimization
	};
} 