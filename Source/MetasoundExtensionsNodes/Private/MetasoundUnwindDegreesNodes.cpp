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

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_UnwindDegreesNode"

namespace Metasound
{
	namespace UnwindDegreesVertexNames
	{
		METASOUND_PARAM(InputValue, "Input", "Input value.");
		METASOUND_PARAM(OutputValue, "Value", "The unwound degrees value.");
	}

	namespace MetasoundUnwindDegreesNodePrivate
	{
		FNodeClassMetadata CreateNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface)
		{
			FNodeClassMetadata Metadata
			{
				FNodeClassName{"UnwindDegrees", InOperatorName, InDataTypeName},
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
		struct TUnwindDegrees
		{
			bool bSupported = false;
		};

		template <>
		struct TUnwindDegrees<int32>
		{
			static void GetUnwindDegrees(const int32 In, int32& OutUnwindDegrees)
			{
				OutUnwindDegrees = FMath::UnwindDegrees(static_cast<float>(In));
			}

			static TDataReadReference<int32> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace UnwindDegreesVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<int32>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}
		};

		template <>
		struct TUnwindDegrees<float>
		{
			static void GetUnwindDegrees(const float In, float& OutUnwindDegrees)
			{
				OutUnwindDegrees = FMath::UnwindDegrees(In);
			}

			static TDataReadReference<float> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace UnwindDegreesVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		template <>
		struct TUnwindDegrees<FTime>
		{
			static void GetUnwindDegrees(const FTime& In, FTime& OutUnwindDegrees)
			{
				OutUnwindDegrees = FTime(FMath::UnwindDegrees(In.GetSeconds()));
			}

			static TDataReadReference<FTime> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace UnwindDegreesVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<FTime>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		// 		template<>
		// 		struct TUnwindDegrees<FAudioBuffer>
		// 		{
		// 			static void GetUnwindDegrees(const FAudioBuffer& In, FAudioBuffer& OutUnwindDegrees)
		// 			{
		// 				TArrayView<float> OutUnwindDegreesView(OutUnwindDegrees.GetData(), OutUnwindDegrees.Num());
		// 				TArrayView<const float> InView(In.GetData(), OutUnwindDegrees.Num());
		// 				
		// 				Audio::ArrayUnwindDegrees(InView, OutUnwindDegreesView);
		// 			}
		//
		// 			static TDataReadReference<FAudioBuffer> CreateInRef(const FBuildOperatorParams& InParams)
		// 			{
		// 				using namespace UnwindDegreesVertexNames;
		// 				const FInputVertexInterfaceData& InputData = InParams.InputData;
		// 				return InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
		// 			}
		// 		};
	}

	template <typename ValueType>
	class TUnwindDegreesNodeOperator : public TExecutableOperator<TUnwindDegreesNodeOperator<ValueType>>
	{
	public:
		static const FVertexInterface& GetDefaultInterface()
		{
			using namespace UnwindDegreesVertexNames;
			using namespace MetasoundUnwindDegreesNodePrivate;

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
				const FName OperatorName = TEXT("UnwindDegrees");
				const FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("UnwindDegreesDisplayNamePattern", "UnwindDegrees ({0})", GetMetasoundDataTypeDisplayText<ValueType>());
				const FText NodeDescription = METASOUND_LOCTEXT("UnwindDegreesDesc", "Returns the angle in degrees unwound to the range -180 to 180.");
				const FVertexInterface NodeInterface = GetDefaultInterface();

				return MetasoundUnwindDegreesNodePrivate::CreateNodeClassMetadata(DataTypeName, OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
			};

			static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
			return Metadata;
		}

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace UnwindDegreesVertexNames;
			using namespace MetasoundUnwindDegreesNodePrivate;

			TDataReadReference<ValueType> Input = TUnwindDegrees<ValueType>::CreateInRef(InParams);

			return MakeUnique<TUnwindDegreesNodeOperator<ValueType>>(InParams.OperatorSettings, Input);
		}


		TUnwindDegreesNodeOperator(const FOperatorSettings& InSettings,
		                           const TDataReadReference<ValueType>& InInput)
			: Input(InInput)
			  , OutputValue(TDataWriteReferenceFactory<ValueType>::CreateAny(InSettings))
		{
			GetUnwindDegrees();
		}

		virtual ~TUnwindDegreesNodeOperator() = default;


		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace UnwindDegreesVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputValue), Input);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace UnwindDegreesVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputValue), OutputValue);
		}

		void GetUnwindDegrees()
		{
			using namespace MetasoundUnwindDegreesNodePrivate;

			TUnwindDegrees<ValueType>::GetUnwindDegrees(*Input, *OutputValue);
		}

		void Execute()
		{
			GetUnwindDegrees();
		}

		void Reset(const IOperator::FResetParams& InParams)
		{
			GetUnwindDegrees();
		}

	private:
		TDataReadReference<ValueType> Input;
		TDataWriteReference<ValueType> OutputValue;
	};

	/** TUnwindDegreesNode
	 *
	 *  Returns the angle in degrees unwound to the range -180 to 180.
	 */
	template <typename ValueType>
	using TUnwindDegreesNode = TNodeFacade<TUnwindDegreesNodeOperator<ValueType>>;

	using FUnwindDegreesNodeInt32 = TUnwindDegreesNode<int32>;
	METASOUND_REGISTER_NODE(FUnwindDegreesNodeInt32)

	using FUnwindDegreesNodeFloat = TUnwindDegreesNode<float>;
	METASOUND_REGISTER_NODE(FUnwindDegreesNodeFloat)

	using FUnwindDegreesNodeTime = TUnwindDegreesNode<FTime>;
	METASOUND_REGISTER_NODE(FUnwindDegreesNodeTime)

	// using FUnwindDegreesNodeAudioBuffer = TUnwindDegreesNode<FAudioBuffer>;
	// METASOUND_REGISTER_NODE(FUnwindDegreesNodeAudioBuffer)
}

#undef LOCTEXT_NAMESPACE
