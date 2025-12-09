// Copyright Hitbox Games, LLC. All Rights Reserved.

#include "MetasoundAudioBuffer.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNode.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundParamHelper.h"
#include "MetasoundPrimitives.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundTime.h"

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_DegreesToRadiansNode"

namespace Metasound
{
	namespace DegreesToRadiansVertexNames
	{
		METASOUND_PARAM(InputValue, "Input", "Input value.");
		METASOUND_PARAM(OutputValue, "Value", "The output value in radians.");
	}

	namespace MetasoundDegreesToRadiansNodePrivate
	{
		FNodeClassMetadata CreateNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface)
		{
			FNodeClassMetadata Metadata
			{
				FNodeClassName{"DegreesToRadians", InOperatorName, InDataTypeName},
				1, // Major Version
				0, // Minor Version
				InDisplayName,
				InDescription,
				PluginAuthor,
				PluginNodeMissingPrompt,
				InDefaultInterface,
				{NodeCategories::Math},
				{},
				FNodeDisplayStyle{}
			};

			return Metadata;
		}

		template <typename ValueType>
		struct TDegreesToRadians
		{
			bool bSupported = false;
		};

		template <>
		struct TDegreesToRadians<int32>
		{
			static void GetDegreesToRadians(const int32 In, int32& OutDegreesToRadians)
			{
				OutDegreesToRadians = FMath::DegreesToRadians(static_cast<float>(In));
			}

			static TDataReadReference<int32> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace DegreesToRadiansVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<int32>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}
		};

		template <>
		struct TDegreesToRadians<float>
		{
			static void GetDegreesToRadians(const float In, float& OutDegreesToRadians)
			{
				OutDegreesToRadians = FMath::DegreesToRadians(In);
			}

			static TDataReadReference<float> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace DegreesToRadiansVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		template <>
		struct TDegreesToRadians<FTime>
		{
			static void GetDegreesToRadians(const FTime& In, FTime& OutDegreesToRadians)
			{
				OutDegreesToRadians = FTime(FMath::DegreesToRadians(In.GetSeconds()));
			}

			static TDataReadReference<FTime> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace DegreesToRadiansVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<FTime>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		// 		template<>
		// 		struct TDegreesToRadians<FAudioBuffer>
		// 		{
		// 			static void GetDegreesToRadians(const FAudioBuffer& In, FAudioBuffer& OutDegreesToRadians)
		// 			{
		// 				TArrayView<float> OutDegreesToRadiansView(OutDegreesToRadians.GetData(), OutDegreesToRadians.Num());
		// 				TArrayView<const float> InView(In.GetData(), OutDegreesToRadians.Num());
		// 				
		// 				Audio::ArrayDegreesToRadians(InView, OutDegreesToRadiansView);
		// 			}
		//
		// 			static TDataReadReference<FAudioBuffer> CreateInRef(const FBuildOperatorParams& InParams)
		// 			{
		// 				using namespace DegreesToRadiansVertexNames;
		// 				const FInputVertexInterfaceData& InputData = InParams.InputData;
		// 				return InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
		// 			}
		// 		};
	}

	template <typename ValueType>
	class TDegreesToRadiansNodeOperator : public TExecutableOperator<TDegreesToRadiansNodeOperator<ValueType>>
	{
	public:
		static const FVertexInterface& GetDefaultInterface()
		{
			using namespace DegreesToRadiansVertexNames;
			using namespace MetasoundDegreesToRadiansNodePrivate;

			static const FVertexInterface DefaultInterface(
				FInputVertexInterface(
					TInputDataVertex<ValueType>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputValue))
				),
				FOutputVertexInterface(
					TOutputDataVertex<ValueType>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputValue))
				)
			);

			return DefaultInterface;
		}

		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
			{
				const FName DataTypeName = GetMetasoundDataTypeName<ValueType>();
				const FName OperatorName = TEXT("DegreesToRadians");
				const FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("DegreesToRadiansDisplayNamePattern", "Degrees To Radians ({0})", GetMetasoundDataTypeDisplayText<ValueType>());
				const FText NodeDescription = METASOUND_LOCTEXT("DegreesToRadiansDesc", "Returns the radian value corresponding to the input degree value.");
				const FVertexInterface NodeInterface = GetDefaultInterface();

				return MetasoundDegreesToRadiansNodePrivate::CreateNodeClassMetadata(DataTypeName, OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
			};

			static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
			return Metadata;
		}

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace DegreesToRadiansVertexNames;
			using namespace MetasoundDegreesToRadiansNodePrivate;

			TDataReadReference<ValueType> Input = TDegreesToRadians<ValueType>::CreateInRef(InParams);

			return MakeUnique<TDegreesToRadiansNodeOperator<ValueType>>(InParams.OperatorSettings, Input);
		}


		TDegreesToRadiansNodeOperator(const FOperatorSettings& InSettings,
		                              const TDataReadReference<ValueType>& InInput)
			: Input(InInput)
			  , OutputValue(TDataWriteReferenceFactory<ValueType>::CreateAny(InSettings))
		{
			GetDegreesToRadians();
		}

		virtual ~TDegreesToRadiansNodeOperator() = default;


		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace DegreesToRadiansVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputValue), Input);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace DegreesToRadiansVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputValue), OutputValue);
		}

		void GetDegreesToRadians()
		{
			using namespace MetasoundDegreesToRadiansNodePrivate;

			TDegreesToRadians<ValueType>::GetDegreesToRadians(*Input, *OutputValue);
		}

		void Execute()
		{
			GetDegreesToRadians();
		}

		void Reset(const IOperator::FResetParams& InParams)
		{
			GetDegreesToRadians();
		}

	private:
		TDataReadReference<ValueType> Input;
		TDataWriteReference<ValueType> OutputValue;
	};

	/** TDegreesToRadiansNode
	 *
	 *  Returns the radian value corresponding to the input degree value.
	 */
	template <typename ValueType>
	using TDegreesToRadiansNode = TNodeFacade<TDegreesToRadiansNodeOperator<ValueType>>;

	using FDegreesToRadiansNodeInt32 = TDegreesToRadiansNode<int32>;
	METASOUND_REGISTER_NODE(FDegreesToRadiansNodeInt32)

	using FDegreesToRadiansNodeFloat = TDegreesToRadiansNode<float>;
	METASOUND_REGISTER_NODE(FDegreesToRadiansNodeFloat)

	using FDegreesToRadiansNodeTime = TDegreesToRadiansNode<FTime>;
	METASOUND_REGISTER_NODE(FDegreesToRadiansNodeTime)

	// using FDegreesToRadiansNodeAudioBuffer = TDegreesToRadiansNode<FAudioBuffer>;
	// METASOUND_REGISTER_NODE(FDegreesToRadiansNodeAudioBuffer)
}

#undef LOCTEXT_NAMESPACE
