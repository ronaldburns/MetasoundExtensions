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

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_SinNode"

namespace Metasound
{
	namespace SinVertexNames
	{
		METASOUND_PARAM(InputValue, "Input", "Input value.");
		METASOUND_PARAM(OutputValue, "Value", "The solved sine of the input.");
	}

	namespace MetasoundSinNodePrivate
	{
		FNodeClassMetadata CreateNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface)
		{
			FNodeClassMetadata Metadata
			{
				FNodeClassName{"Sin", InOperatorName, InDataTypeName},
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
		struct TSin
		{
			bool bSupported = false;
		};

		template <>
		struct TSin<int32>
		{
			static void GetSin(const int32 In, int32& OutSin)
			{
				OutSin = FMath::Sin(static_cast<float>(In));
			}

			static TDataReadReference<int32> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace SinVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<int32>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}
		};

		template <>
		struct TSin<float>
		{
			static void GetSin(const float In, float& OutSin)
			{
				OutSin = FMath::Sin(In);
			}

			static TDataReadReference<float> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace SinVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		template <>
		struct TSin<FTime>
		{
			static void GetSin(const FTime& In, FTime& OutSin)
			{
				OutSin = FTime(FMath::Sin(In.GetSeconds()));
			}

			static TDataReadReference<FTime> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace SinVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<FTime>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		// 		template<>
		// 		struct TSin<FAudioBuffer>
		// 		{
		// 			static void GetSin(const FAudioBuffer& In, FAudioBuffer& OutSin)
		// 			{
		// 				TArrayView<float> OutSinView(OutSin.GetData(), OutSin.Num());
		// 				TArrayView<const float> InView(In.GetData(), OutSin.Num());
		// 				
		// 				Audio::ArraySin(InView, OutSinView);
		// 			}
		//
		// 			static TDataReadReference<FAudioBuffer> CreateInRef(const FBuildOperatorParams& InParams)
		// 			{
		// 				using namespace SinVertexNames;
		// 				const FInputVertexInterfaceData& InputData = InParams.InputData;
		// 				return InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
		// 			}
		// 		};
	}

	template <typename ValueType>
	class TSinNodeOperator : public TExecutableOperator<TSinNodeOperator<ValueType>>
	{
	public:
		static const FVertexInterface& GetDefaultInterface()
		{
			using namespace SinVertexNames;
			using namespace MetasoundSinNodePrivate;

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
				const FName OperatorName = TEXT("Sin");
				const FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("SinDisplayNamePattern", "Sin ({0})", GetMetasoundDataTypeDisplayText<ValueType>());
				const FText NodeDescription = METASOUND_LOCTEXT("SinDesc", "Returns the sine of the input value.");
				const FVertexInterface NodeInterface = GetDefaultInterface();

				return MetasoundSinNodePrivate::CreateNodeClassMetadata(DataTypeName, OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
			};

			static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
			return Metadata;
		}

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace SinVertexNames;
			using namespace MetasoundSinNodePrivate;

			TDataReadReference<ValueType> Input = TSin<ValueType>::CreateInRef(InParams);

			return MakeUnique<TSinNodeOperator<ValueType>>(InParams.OperatorSettings, Input);
		}


		TSinNodeOperator(const FOperatorSettings& InSettings,
		                 const TDataReadReference<ValueType>& InInput)
			: Input(InInput)
			  , OutputValue(TDataWriteReferenceFactory<ValueType>::CreateAny(InSettings))
		{
			GetSin();
		}

		virtual ~TSinNodeOperator() = default;


		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace SinVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputValue), Input);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace SinVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputValue), OutputValue);
		}

		void GetSin()
		{
			using namespace MetasoundSinNodePrivate;

			TSin<ValueType>::GetSin(*Input, *OutputValue);
		}

		void Execute()
		{
			GetSin();
		}

		void Reset(const IOperator::FResetParams& InParams)
		{
			GetSin();
		}

	private:
		TDataReadReference<ValueType> Input;
		TDataWriteReference<ValueType> OutputValue;
	};

	/** TSinNode
	 *
	 *  Returns the sine of the input value.
	 */
	template <typename ValueType>
	using TSinNode = TNodeFacade<TSinNodeOperator<ValueType>>;

	using FSinNodeInt32 = TSinNode<int32>;
	METASOUND_REGISTER_NODE(FSinNodeInt32)

	using FSinNodeFloat = TSinNode<float>;
	METASOUND_REGISTER_NODE(FSinNodeFloat)

	using FSinNodeTime = TSinNode<FTime>;
	METASOUND_REGISTER_NODE(FSinNodeTime)

	// using FSinNodeAudioBuffer = TSinNode<FAudioBuffer>;
	// METASOUND_REGISTER_NODE(FSinNodeAudioBuffer)
}

#undef LOCTEXT_NAMESPACE
