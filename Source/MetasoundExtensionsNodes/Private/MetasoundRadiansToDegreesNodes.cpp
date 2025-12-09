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

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_RadiansToDegreesNode"

namespace Metasound
{
	namespace RadiansToDegreesVertexNames
	{
		METASOUND_PARAM(InputValue, "Input", "Input value.");
		METASOUND_PARAM(OutputValue, "Value", "The value in degrees corresponding to the input radian value.");
	}

	namespace MetasoundRadiansToDegreesNodePrivate
	{
		FNodeClassMetadata CreateNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface)
		{
			FNodeClassMetadata Metadata
			{
				FNodeClassName{"RadiansToDegrees", InOperatorName, InDataTypeName},
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
		struct TRadiansToDegrees
		{
			bool bSupported = false;
		};

		template <>
		struct TRadiansToDegrees<int32>
		{
			static void GetRadiansToDegrees(const int32 In, int32& OutRadiansToDegrees)
			{
				OutRadiansToDegrees = FMath::RadiansToDegrees(static_cast<float>(In));
			}

			static TDataReadReference<int32> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace RadiansToDegreesVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<int32>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}
		};

		template <>
		struct TRadiansToDegrees<float>
		{
			static void GetRadiansToDegrees(const float In, float& OutRadiansToDegrees)
			{
				OutRadiansToDegrees = FMath::RadiansToDegrees(In);
			}

			static TDataReadReference<float> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace RadiansToDegreesVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		template <>
		struct TRadiansToDegrees<FTime>
		{
			static void GetRadiansToDegrees(const FTime& In, FTime& OutRadiansToDegrees)
			{
				OutRadiansToDegrees = FTime(FMath::RadiansToDegrees(In.GetSeconds()));
			}

			static TDataReadReference<FTime> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace RadiansToDegreesVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<FTime>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		// 		template<>
		// 		struct TRadiansToDegrees<FAudioBuffer>
		// 		{
		// 			static void GetRadiansToDegrees(const FAudioBuffer& In, FAudioBuffer& OutRadiansToDegrees)
		// 			{
		// 				TArrayView<float> OutRadiansToDegreesView(OutRadiansToDegrees.GetData(), OutRadiansToDegrees.Num());
		// 				TArrayView<const float> InView(In.GetData(), OutRadiansToDegrees.Num());
		// 				
		// 				Audio::ArrayRadiansToDegrees(InView, OutRadiansToDegreesView);
		// 			}
		//
		// 			static TDataReadReference<FAudioBuffer> CreateInRef(const FBuildOperatorParams& InParams)
		// 			{
		// 				using namespace RadiansToDegreesVertexNames;
		// 				const FInputVertexInterfaceData& InputData = InParams.InputData;
		// 				return InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
		// 			}
		// 		};
	}

	template <typename ValueType>
	class TRadiansToDegreesNodeOperator : public TExecutableOperator<TRadiansToDegreesNodeOperator<ValueType>>
	{
	public:
		static const FVertexInterface& GetDefaultInterface()
		{
			using namespace RadiansToDegreesVertexNames;
			using namespace MetasoundRadiansToDegreesNodePrivate;

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
				const FName OperatorName = TEXT("RadiansToDegrees");
				const FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("RadiansToDegreesDisplayNamePattern", "Radians To Degrees ({0})", GetMetasoundDataTypeDisplayText<ValueType>());
				const FText NodeDescription = METASOUND_LOCTEXT("RadiansToDegreesDesc", "Returns the degree value corresponding to the input radian value.");
				const FVertexInterface NodeInterface = GetDefaultInterface();

				return MetasoundRadiansToDegreesNodePrivate::CreateNodeClassMetadata(DataTypeName, OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
			};

			static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
			return Metadata;
		}

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace RadiansToDegreesVertexNames;
			using namespace MetasoundRadiansToDegreesNodePrivate;

			TDataReadReference<ValueType> Input = TRadiansToDegrees<ValueType>::CreateInRef(InParams);

			return MakeUnique<TRadiansToDegreesNodeOperator<ValueType>>(InParams.OperatorSettings, Input);
		}


		TRadiansToDegreesNodeOperator(const FOperatorSettings& InSettings,
		                              const TDataReadReference<ValueType>& InInput)
			: Input(InInput)
			  , OutputValue(TDataWriteReferenceFactory<ValueType>::CreateAny(InSettings))
		{
			GetRadiansToDegrees();
		}

		virtual ~TRadiansToDegreesNodeOperator() = default;


		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace RadiansToDegreesVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputValue), Input);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace RadiansToDegreesVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputValue), OutputValue);
		}

		void GetRadiansToDegrees()
		{
			using namespace MetasoundRadiansToDegreesNodePrivate;

			TRadiansToDegrees<ValueType>::GetRadiansToDegrees(*Input, *OutputValue);
		}

		void Execute()
		{
			GetRadiansToDegrees();
		}

		void Reset(const IOperator::FResetParams& InParams)
		{
			GetRadiansToDegrees();
		}

	private:
		TDataReadReference<ValueType> Input;
		TDataWriteReference<ValueType> OutputValue;
	};

	/** TRadiansToDegreesNode
	 *
	 *  Returns the degree value corresponding to the input radian value.
	 */
	template <typename ValueType>
	using TRadiansToDegreesNode = TNodeFacade<TRadiansToDegreesNodeOperator<ValueType>>;

	using FRadiansToDegreesNodeInt32 = TRadiansToDegreesNode<int32>;
	METASOUND_REGISTER_NODE(FRadiansToDegreesNodeInt32)

	using FRadiansToDegreesNodeFloat = TRadiansToDegreesNode<float>;
	METASOUND_REGISTER_NODE(FRadiansToDegreesNodeFloat)

	using FRadiansToDegreesNodeTime = TRadiansToDegreesNode<FTime>;
	METASOUND_REGISTER_NODE(FRadiansToDegreesNodeTime)

	// using FRadiansToDegreesNodeAudioBuffer = TRadiansToDegreesNode<FAudioBuffer>;
	// METASOUND_REGISTER_NODE(FRadiansToDegreesNodeAudioBuffer)
}

#undef LOCTEXT_NAMESPACE
