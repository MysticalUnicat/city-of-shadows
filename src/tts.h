/*

forward:
  input input_ids              u32[]
  input attention_mask         bool[]
  input input_values           float[]
  input padding_mask           bool[]
  input decoder_input_ids      u32[]
  input decoder_attention_mask bool[]
  input encoder_outputs        float[][]
  input past_key_values        float[][]
  input decoder_inputs_embeds  float[]
  input prompt_input_ids       float[]
  input prompt_attention_mask  u32[]
  input prompt_hidden_states   float[]
  input decoder_position_ids   u32[]
  input labels                 u32[]
  input use_cache              bool
  input output_attentions      bool
  input output_hidden_states   bool
  input return_dict            bool
  input cache_position         u32[]

generate:
  input inputs              tensor = none
  input generation_config   logits_processor_list = none
  input stopping_criteria   stopping_criterial_list = none
  input synched_gpus        bool = none
  input streamer            BaseStreamer = none

parler-1.0-mini

ParlerTTSForConditionalGeneration(
  text_encoder: T5EncoderModel(
    shared: Embedding(32128, 1024)
    encoder: T5Stack(
      embed_tokens: Embedding(32128, 1024)
      block: ModuleList(
        T5Block(
          layer: ModuleList(
            T5LayerSelfAttention(
              SelfAttention: T5Attention(
                q: Linear(in_features=1024, out_features=1024, bias=False)
                k: Linear(in_features=1024, out_features=1024, bias=False)
                v: Linear(in_features=1024, out_features=1024, bias=False)
                o: Linear(in_features=1024, out_features=1024, bias=False)
                relative_attention_bias: Embedding(32, 16)
              )
              layer_norm: T5LayerNorm()
              dropout: Dropout(0.1, inplace=False)
            )
            T5LayerFF(
              DenseReluDense: T5DenseGatedActDense(
                wi_0: Linear(in_features=1024, out_features=2816, bias=False)
                wi_1: Linear(in_features=1024, out_features=2816, bias=False)
                wo:   Linear(in_features=2816, out_features=1024, bias=False)
                dropout: Dropout(0.1, inplace=False)
                act: NewGELUActivation()
              )
              layer_norm: T5LayerNorm()
              dropout: Dropout(0.1, inplace=False)
            )
          )
        )
        23 x T5Block(
          layer: ModuleList(
            T5LayerSelfAttention(
              SelfAttention: T5Attention(
                q: Linear(in_features=1024, out_features=1024, bias=False)
                k: Linear(in_features=1024, out_features=1024, bias=False)
                v: Linear(in_features=1024, out_features=1024, bias=False)
                o: Linear(in_features=1024, out_features=1024, bias=False)
              )
              layer_norm: T5LayerNorm()
              dropout: Dropout(0.1, inplace=False)
            )
            T5LayerFF(
              DenseReluDense: T5DenseGatedActDense(
                wi_0: Linear(in_features=1024, out_features=2816, bias=False)
                wi_1: Linear(in_features=1024, out_features=2816, bias=False)
                wo:   Linear(in_features=2816, out_features=1024, bias=False)
                dropout: Dropout(0.1, inplace=False)
                act: NewGELUActivation()
              )
              layer_norm: T5LayerNorm()
              dropout: Dropout(0.1, inplace=False)
            )
          )
        )
      )
      final_layer_norm: T5LayerNorm()
      dropout: Dropout(0.1, inplace=False)
    )
  )
  audio_encoder: DACModel(
    model: DAC(
      encoder: Encoder(
        block: Sequential(
          Conv1d(1, 64, kernel_size=7, stride=1, padding=3)

          EncoderBlock(
            block: Sequential(
              ResidualUnit(
                block: Sequential(
                  Snake1d()
                  Conv1d(64, 64, kernel_size=7, stride=1, padding=3)
                  Snake1d()
                  Conv1d(64, 64, kernel_size=1, stride=1)
                )
              )
              ResidualUnit(
                block: Sequential(
                  Snake1d()
                  Conv1d(64, 64, kernel_size=7, stride=1, padding=9, dilation=3)
                  Snake1d()
                  Conv1d(64, 64, kernel_size=1, stride=1)
                )
              )
              ResidualUnit(
                block: Sequential(
                  Snake1d()
                  Conv1d(64, 64, kernel_size=7, stride=1, padding=27, dilation=9)
                  Snake1d()
                  Conv1d(64, 64, kernel_size=1, stride=1)
                )
              )
              Snake1d
              Conv1d(64, 128, kernel_size=4, stride=2, padding=1)
            )
          )

          # same thing as above, but 128 to 256
          # same thing as above, but 256 to 512
          # same thing as above, but 512 to 1024
        )
      )
      quantizer: ResidualVectorQuantize(
        9 x VectorQuantize(
          in_proj: Conv1d(1024, 8, kernel_size=1, stride=1)
          out_porj: Conv1d(8, 1024, kernel_size=1, stride=1)
          codebook: Embedding(1024, 8)
        )
      )
      decoder: Decoder(
        model: Sequential(
          Conv1d(1024, 1536, kernel_size=7, stride=1, padding=3)
          DecoderBlock(
            block: Sequential(
              Snake1d()
              ConvTranspose1d(1536, 768, kernel_size=7, stride=1, padding=3)
              ResidualUnit(
                block: Sequential(
                  Snake1d()
                  Conv1d(768, 768, kernel_size=7, stride=1, padding=3)
                  Snake1d()
                  Conv1d(768, 768, kernel_size=1, stride=1)
                )
              )
              ResidualUnit(
                block: Sequential(
                  Snake1d()
                  Conv1d(768, 768, kernel_size=7, stride=1, padding=9, dilation=3)
                  Snake1d()
                  Conv1d(768, 768, kernel_size=1, stride=1)
                )
              )
              ResidualUnit(
                block: Sequential(
                  Snake1d()
                  Conv1d(768, 768, kernel_size=7, stride=1, padding=27, dilation=9)
                  Snake1d()
                  Conv1d(768, 768, kernel_size=1, stride=1)
                )
              )
            )
          )
          # same as above but 768 to 384
          # same as above but 384 to 192
          # same as above but 192 to 96
          Snake1d()
          Conv1d(96, 1, kernel_size=7, stride=1, padding=3)
          Tanh()
        )
      )
    )
  )
  decoder: ParlerTTTSForCausalLM(
    model: ParlerTTSModel(
      decoder: ParlerTTSDecoder(
        embed_tokens: ModuleList(
          9 x Embedding(1089, 1024)
        )
        embed_positions: ParlerTTSinusoidalPositionalEmbedding()
        layers: 24 x ParlerTTSDecoderLayer(
          24 x ParlerTTSDecoderLayer(
            self_attn: ParlerTTSdpaAttention(
              k_proj: Linear(1024, 1024)
              v_proj: Linear(1024, 1024)
              q_proj: Linear(1024, 1024)
              out_proj: Linear(1024, 1024)
            )
            activation_fn: GELUActivation()
            self_attn_layer_norm: LayerNorm(1024, eps...)
            encoder_attn: # same as self attn
            encoder_attn_layer_norm: LayerNorm(1024, ...)
            fc1: Linear(1024, 4096)
            fc2: Linear(4096, 1024)
            final_layer_norm: LayerNorm(1024, ...)
          )
        )
        layer_norm: LayerNorm(1024, eps=1e-05, elementwise_affine=True)
      )
      lm_heads: ModuleList(
        9 x Linear(1024, 1088)
      )
    )
  )
  embed_prompts: Embedding(32128, 1024)
)

unfinished_sequences = float[]{1.0 ...}
model_kwargs = 

*/

