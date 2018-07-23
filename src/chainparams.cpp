// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

std::string
baseAddr[GENESISCOIN_CNT] = {"04b297c42453abaa7ecbc609a4a12cbea0b033c8a13336e680811151ddd36e3021cd3f7a2daba1162ee760b2195d3b34fdff0b69869c2f972514bfce3afc4ffd3b",
                             "046ac572813c8546b358b7e7a980142d323ae5223c74374a965f8131368f48c955b69157a685a9659075c2e409cd2abe301eaac437022a446b813a929434c049ee",
                             "04e9f02da5c74d5c4b70a16b0499fbfa35b71f459e20eb961d67814e8c95406bb7dfe1b2a9a4174fce0af44d1e34515ad6fc507e795def25d0940c45cebcfc1c8d",
                             "04873c75f59fb77a4b5e9c89f36e95f1881f4a31004fdd561742d32e1ef036c6c3bf9221442bd1a568b528b01eb7ab38478fddd291a1a96c245182bf56536c25bf",
                             "04eaf351b8722eb02142e56a7381da13351aefad1bc1dc7ef537ef88cbdeab5dcfb8ad795d27a543ff2e8ec2526d032a3c1d306e23e6cfd82f2166814818ff88aa",
                             "0493ccb21b163d202a203c47cd91abb6d03890fb609bb798dbccc6d84e009263a6a72f31becf43ec4dc24464c6e25844a8412bf0a364f00cb747bb0f99aacace4d",
                             "04958ccb6a4dd8d11a4c5dc53ec3249f5ef2bd4c09b915c84d3fd197ac678c859ee528e19c6535b40f7a673950c7d5e31435b12d319c51650036cbe0282df610e9",
                             "04e81a00f15e784154fcdc225c7f7fb374aa07365b3db5d61d241632ff78012f7e60cf6c0851913d5b91dae3a2093f29013c728986a49e32d81807c6bf8208794b",
                             "047b934d7f9dae9f2ac8f35b39a95d074d209b290d984ec34b53b35ec56be5df191b64638f1c51b1d89135013c9654581dca1b028e0dc512fc571881ab4fbf3701",
                             "04066484a4e925176503f480e99bd5f5268b154155aba0d95680485a73dd4cf4d0958d4b440d89833fa390f32d0c710f1a45d678b4001536e58a6b832decf72bf9",
                             "04f72cbe79b91ba7db3208107d7952728cddf519d4741c0fc4dd06bec86dffad0c58d25266fc0cdcaa3a392440328b9294250a6f881093a51dce9394460453d239",
                             "042c5eec75b244d1204466bdf542c827444922cf8d96b1024138c3d0bb9b8aaa6dba7e7edd02a702f266eb9431388a5ec0a098a56916abd1fe4656045510cf7f11",
                             "04c685a9ba59016faa72340d4eca18b5882263842c8982b3bc6143aff9ec969d047adf6c3cba8cff7c6da8d4f6f31511e8677c826406f0147598f3e1af7504aac1",
                             "04532c6b90dd309014a23be7640809f5db88adb2f6c9bd8b98549868f5197a25d70e4df32a6f8127d46e513b781a3048efaa1a361a07edbe785231f9e0f2d0c5f3",
                             "04ec2720e4269d5740d0edf9b509900bee0b350732d82fe920e22c58a420d7ffa86da573d4fb5a0e7687ed011d21949a0c05deb9c56f7801a5a06869ea4af3ce1b",
                             "04893f1f1d20238abe887e8e70d63598c2065d2011492d373ceea0b7abff25f23365038d81f036d88d13ac1ecd21b17382fae0c975f5e49111730f1aba6381ce5d",
                             "0477c99786a86db89e0aa6928c5d1a038f77b9cd07879ad191674fbb2286260a36edd2dcb1ce47a7f05cde6d354efd3a47cc82451b839cb195118452e4b0a8a72e",
                             "042526384d5076fce2d34af8c9c5d729ef63a132e0dacd6a0eedf7157dbe7aa7eadbe10bfaff6d5a4dca4424063e9f6b51b32083b4f45564dfd7101fb7c9c7ea7a",
                             "04c335d9fdfdb0e9780bb430cc1a3dfd2a04a1be0182bb4ac396a0b2ab6ace7eb5f86ed9d2275d335966f212a8e15e7ae2f9a3af16bd2fedc259796155e32f65aa",
                             "047a5116e6edbdda523a50736bd0c88b90563cbe25e57e87e7d93406363c319b38fc5e9d2ad5bfb8d2c8d8e509e80d3f6f247cf65e63187da5db6cc42b4dffb85f",
                             "040326c9e8861c42bf0adf66e6276fe6c735988960dd98574784dc864bc96b3c6d3bc21feff590c3b702f86ff6a07c648b64caf22acd3291e7eed23618dac34279",
                             "0442baf3ffc077ee15aba4700e8b5b89b12cb7dca1a70ea58ef5a4d894d649b203a32cb9c3cf177f896acbea10a406c548c60ae57839ff002cb69a3b06424a526e",
                             "04c8dac1f1a2706173adb6cfff01b01970786dcb8432b6ba1f32d3aca34fd2af8a642fb8b3e2262fcf1035ae2c73187a4e217b18182fcfc9bee87c3527a1052e8b",
                             "04be593ac36bd55ad77c8812e113fcb706de27d7e7f7d59278c9ea42b80c96bef450de806efb9c4556924c7c182ea294e5c8fd1baec8cfc88b7b46cda7fdb5cbf8",
                             "04b13e9f4f458739a8b97d7903bd8fee6769b84f2614270bba7875a587b57f5a88a712557232b2285acdb1a4aefd93bd3c72c9cc33016515ba77f1d960853be507",
                             "0426e54b17b6ecb5750250971a0722209d5533cd74e3cc08ce7adb638a0a1e66a596de60440573f837a4388f0ae1f1179975100998e04c8e689547dc5780cfb5d5",
                             "04fc385f4f01d2fecdf9da44f13b5c817ae03abb5708950b09ac64089234ec06a6775219c4f4ecf4625bebb1ec60a97e6e49fdfb42de903328b26d296326b7e8a1",
                             "043c55d78a4d559044a0abb2e540fbb12423f1b274df73f4e29261da5dc111568e4a2b9d25659646a7f386824e548f4eb8e335dfcc66beb21fd67cd25a5202becd",
                             "04e3462f8b0d4bc143e2ea49d4f7f474d05df75d101c1eac8f075588c35ca337563e2425b6af50ed9472e93d5d44eb711c9dc4e1b7797acc5f67e596d359719407",
                             "044b79c03ad099d1af9690432378c0253debeb7146ec80446f0e03fac96d4300c9b70d98ad598353486d6218cb50551c49ea19f72a327762e2da67c0acdba6b24a",
                             "042fa718e9352057b56df697038ec514f9a0a46d6c9ff6c000e7951d5178bef4f25bebb0198415b52bd002f75a569cdff0d4016f8d6a68a626f9b742d7ed9c92a1",
                             "048351b76ba7d5bc6b40e5b373edc413c3594e35cca6ae067b17791ceb69f3490e6361acd4fe8fd90b1b64da6d43218ad201f8bf2c7023d7563c792517b3ab2a77",
                             "048cd27ffececea24165cbf03de50bd7f9a7d0f2cdb4bdc9783d7d6203df19ebc37cee8438e98bf0c4994aaddc55ea8921393eacb057a26e02a574a90b539ae4d6",
                             "049073808ee0f3fd3e044acc03484653cb218d5ec5b0643de590aa717b04e10d71e04bece76965b44cced3e7304dfbf045b53362931b68074116076f2202378104",
                             "045012ffe7c73bb61597bdb9a0ce130ae9d8b0f9dfaccb4207fba1dfe0a50e5ac981ea05e7d79038b2e684329c8b4c8b51160570f7d69f4668b63afc7813ad1228",
                             "04fb2d61d6ca0afbcc79e34a6ae65b4ae204f65fff1924b1b96af06beab6c6e29f2565e0926e9f60fc7a51fc3a72162b3d1075c5385efa2c8eed7c0b69df36dc26",
                             "042cf9abf18f7ed31ecfdfe56ac6601b49a823aa5e2827cce853e073332ba1a80385228c44bd1592874852093049092a311579bd488dca982f57f2189e1fa7d7f0",
                             "04e321893edd1595a206efff04fca9e92a07005ef35d11a1319c0646018b64f92218e7af15b5773aadeb7dbf504bda3474935d85681d2578e85ec8fd315022f250",
                             "04aa7ac2407d248075757da7b93e243062ecc6cf37e3b337e8b0a256aea082604e5eee99f5839591be640ca5348df3619687814117d4ce07a5fd9b255e95bc075f",
                             "041ceca7157b032db292ee52ec8c0ead3eada34d0d88d27527d75968b9b38fe4d3afbb12e2571837d6bdd3826898a0ea02d99cf65ac6b979a00571edec330afb83",
                             "04c92e12dcb38046827ca5c84d94907129842f83a007149639d0e62424431e4f918c64c589aba0a1f718f10d2a5135e56701fbb27d8991bb9259bd62f7fb48c46a",
                             "0482edfe1931f32c037044c3999924750972158a5bbe8417a5c955d06801d14a132327569d5ac1a9674f3649fca073fed6120f71cc6a0d0523a496f65fead2af21",
                             "04a36b8df6fb07cefee6b16ea8600978edf4db46f5de0805e96c62f1f14b57cbe27cdfea3c7330e8098d208d5f131b26455f952efa33fff27c181cc7e9b4900354",
                             "049b738ed2a2b88929cd27c4a03ed91a83687ac109011d69250f37a842bc7e32e4605b05d77efefff3f70d8b23a5157a36f08110eebb066d2a2e50e8a2096d8922",
                             "0499187f1de44b3ee533df98bcc8a387ddd31108a40ee9d4c6ec5811311b43daf159391230ee06e6a0394b0e33e943cad75ea1452c21f81dc9fe0d272282d69ab1",
                             "047d7282562f5e8dbb67691bf7a397be97dabb840fa0fc1867e4ce4c18146c222cf47a4384ff45c4cdd5c0ad28332233490a249c88fa1470afe506cb8994a3651e",
                             "0460cab15e38d18cf598182f7d724161c0c26d1d8d57025950a746f7dd07d8871bc5bbebc2724274b1a06af49a2b223896d657d6b4c5c98f6a167070b58da53a10",
                             "04d711f7e6cd380506c377342b6071899094f9d1ea87d6af0cfd071d36fc7169f5b2cfd612803e37048dbcb0a7b71fcc528d5f122b65cc24f9870cd0ad36177fb8",
                             "042e278b9da480de1826fc579605d14c943281d435e5d5a57cb67270dfdadcce8a2ddadc12c40401c235022db69cce7c904853bf1d73df84ae3ae9de0d47d49b2c",
                             "0455128d3e46567794bd3cff683d2d7d919814df45c0df1d6dca19c8f7c48fb5ec489aff57b73f9cdb3c8b011adbc5e997ea892ba20813b90f5da23c3e48e64774",
                             "0480243e6dd248d8b4af9e0224593d1d5c6e9cb2d64415220857d6b0cc7573621c088c3b00a9671f88038eb0ec89cf7333dbe2b1729126e0de49f358ed56425fb9",
                             "0483522dc8328b539a444f96cb20b7a962ce4ed0cc876b84d432b880c4b28e32e2301f700106bfd0fbb43c3d77785b223f88dcf47040d41b5c65129383dcc2bec0",
                             "04f5d64b47f1cbefbbed6784dedf8752c044f6749755e0e10872b4e115f4a5803c48fc29f899ee62bd2a1a9a9c1270058ace9528036160acc5e98c56385160181c",
                             "048bf6c74ca9f652c96870874bc4540a83b92114a5d15dba637ee5329d634d0cbb8effc27687131ba94ac6c8c02e7ab0a4c293fb80cf208935399986a955a6ebbc",
                             "04a2d44d780fdd0bae6215dc65987f5cc711d08a4aa7291db2c8a3635a899836ae956e14cb155e992f12d8e9a3d21275eb9e56c4de14824e0cce6f759b1d536c5f",
                             "04d5cdcfb48feb25e560887be69e0f1bad8a5982865c9ab70d69e9999dd8924d5d538fae1b15bf3578fe1e936acc9f2d06a3f24953c57d4c0563528d74ad645e57",
                             "0497580113626c3135e4609babc3b148b6dabada31b59482c1705e73a85ae81e6127f9971f2bf12dc49534fbf1cbacc6c87cd4394bc953a6498eecd848817d1683",
                             "048b277c51b697de9576627e21e42a3d6b75b41565a6fd4ac018621564bff0e4b2e89b720afc2c66fb137ff3862b58fe9a525cf1226d1b1622b0b1035be81b793f",
                             "04ae420477489554a7789855787c19b9650cbac6e748cd71ffd83d41b1b7a86df38c835624631af2fd609afba530f123d42c9dc7365832276c08b1ef81e6e74c93",
                             "0494676f69cc51f20b7d6e7defeaac4297ef6e120d17b7a26b7b5d3f84db590d31a31db10890004d4567ab3da509bf18438fa1d6d4a45a01392467f556cabf8c47",
                             "04c3984a6e13cd823e28991a7e5ed1c738ebed28c247354d43f8ac35f9e25e8522050f14dabf9c58b315bce3b2f0e4b50f67bf599af48dcc7a0f80e918cb7f5436",
                             "04e178b9c8bada9b97c57e1cc9f7bf5e36b0461d1dcd5a3f8a755d34f53d24f6f27cef12e7520f4352ea388fd7920e001bedb675517afd08ae910e5ce569c28953",
                             "04565a49b11c189911c29c19d2756ae6a4a8cbcd61039286806732b96b6b7ff2bcb5ada9fa1023f5a7231712c5eabf8dafc7e9fa06470ed5e6889907752a8d7745",
                             "0487548280b2eec7c5d7eb3f8ebd9fe00a4ebb4232ddfbdc4a5cf159cab7861c124e5af2d15ab8df0df6ac654f65f2a5bfe41feafff657efe88706983d2617b9d8",
                             "04189294158cecd7773f4cfca14d0197c36d0d8143fa0777af7608b658237146d78e5f0ab0330e425d97ca51fc44c8c4975f937598d886b41eefbe74ec40b58071",
                             "04e1a3439a2e134da2ca992c33c5ee2d5fe43b70f2dabb6cec99e17faf871c5379a18fa97c76397cd42479dc99c780e287484be48ad01fb9c0bcc7e4b5d68da9b5",
                             "043c77e3ff7bc6be235086ee15dccc03a4116acc7d76b0f43a09c6fcde9746bcfa337bf92d92669caa702d45d8540ebce988cdf6f21fb03ab9427d5eff5cf5a962",
                             "048d11b297b651c95dbbb5ecf628791b32788eb058843727333cfe77f05f29134189123781caee347bc6700ca1b04043317e194caf4e624f3dd3348e67d8fb45f9",
                             "04b2009d75a68a665d8b0814682809387b8c2fd102f19fe5a2bce681218834cd8a042d95333ba3b18953ea4219f5e2ba73f6a825f2b67c2b4f78808dbf4d236ef9",
                             "04c35b3c4d5d07d34303d74dd16e52f64eb5ddae15bbbdaefb00d9efab49c439b806d800a7f9367618d84a8caf233d4f1214396a867005b97f2dd5cc0a611868e1",
                             "04387df20674ac202492457624387e6e95a3cbd57674afd1be32748244bcc5288af26d14d0f7dcd21e76dbc1765a99e0e2cb5ba5a634c6d89aa18840f17e0c6391",
                             "046b29a68abdeb481d98cfa60fc6328079ba5997517d73b2abf579c5071b84bd4462c96ea89b8866748a1d2f990218b166afd44e08959fbc587379fa6076f85313",
                             "04b6624cfaae3c9590616f4422afa81c936cd8ceb2804dbf226f923d9cd3864a120781d588e303c180afa0d904f10766413d23ddb955813603540e1b59f25f002e",
                             "040d830dbbe166b97d73f6712b079a38a21197b32bc741997a44ebefeb40dcbf817a53cef25c8891c160f45741a4e854ef46ebe52ed956cab49e66f5c3e7f51d9c",
                             "046596696dfaddf782118f65ab6456e777aa6ec71c6290fa49a1e21c6fa162a5129e8e823bbc7a8c9d4e6c1fcc37d36ac92f4fd31f2fb30e77f2a82dfd346e9daf",
                             "041064cb314c641c11d9b87871b81d0c8311be13a3c84433d99837dccc67173864b3ea297b2199b08ad7c148ea474e92d5e3b4bb2e1b5d7e1d4f6366c6aa4fa8e7",
                             "0492201b071773c3fe089f21b0e6414e0ab7943d15e04112bffa1f4eade4612b06092ae0aac5b705257974c4529cce53a3d0c77315bdff04b992810fcc9a44264c",
                             "0419e96adf93ea67988531851878ae1f53fff82299a29418d5d4789c5846e27b1109e787a1c17fb6a35ea227f58fba8999e5162abc840fc6ab202a5b8f3063de51",
                             "048d4981afdba67f19d9cac4be098faf1b5be770049ab72c0e780914d7d715c4c2e78e7714480dc1dbafd77f06401cc845b56a1371a3c3e88a9b28a62870654e50",
                             "0456e1b96aaecab27e41b31e80a35e9b2429d999058a176eb0682d703dacd9e49c54030d8779bcdd862371eae89c602711a4ce4050cd245d29aff41272d97bbd3a",
                             "04966b7c5b5ab83533d409e3ccc2d77504d89abbcd110983974c79901a36fd1e1633b05f0dfb222258c09601eb9afa372f43658a1071f144fdea456c7f9d78a1cd",
                             "043969f03a2bc0463fc7e17ab509a6b6fbc1051111765bde9f072ab959c5a5c1eb60b214122ec72783c19acae2ebce4e1d08fcdc5206edda377d99057cac604c34",
                             "04b03beeff3f74335b04c567910dce0687377fff86467e23b2598cc42b097fc3f235b9b1e59b884edd35d9713e242216930eaa513f20a0d9bb27c8cd3bc2ad6295",
                             "040aa33f20b29f6c1fdca2fdd34f9589ac55bee4be7834c4936826f7201263bd0e34d3ae1b93c46c19c17ac3ac106426a5cae6a897a69fca7c3681e078c9075778",
                             "04ac6b6702f64f909d73e3f52377fb994b09560e0fcdebcf8542a6468b7430a3d4d33db4f644f03ed4303f1a96beff95a8a9d81269f3f9396849f95d45b6010a22",
                             "04560a2c989593bfda1e3739b4ec796b0cdcf1efaa1b810e1d2541915a8e909bba1b90a3a062a9f06151d0335572ca466526e9245a508407a8f6eb0fa9be50e16b",
                             "042d59ee25ea711cea165fe8d8a7bb838f0f2ef80b08890a6063026d23771af7a0ae8a8c0b4fcca47e81fa7d875aefbe6ccbd305757c45577274f6e86392fd9a39",
                             "0452b7bb586af0e3d0ed646b0efa95a5ca3c190dd9b07ae50273dbec002bd4d9a206986afb12635c722e281f3f27f82f9fe1e4a47817bf14559e85c64b74a2da9b",
                             "04863809aeeb81d86c2b653783b5a77f20ea12dedffc875efbb4cdddbd299ec3771250532329df9b9f1a62c3cd59f376432166cc1145460c5c69cb6e584fd95419",
                             "0497d5eda301266818946838aa38e6998d00358e4b089860390939c4017057e4cb1bb5df165354132611f29ff81e45d87966c21321c417c1f10dc89c4dbe629a76",
                             "042e4414fd3d328a4ba6e71b244163d35fe78643e33cf94d273ce721829f663b69c42ef05dd92e13c9b916180f9dbd7566136bfffd36a5e3a617ef10849eae1326",
                             "0443c74fd83d8977a30da5205bd790a7e3772ac9eb989a50e37429013dc0952677fa78a81845e2b44083d49b185067eeecbadc9ff72183c4a84eb74c5a0b86525d",
                             "04da213fc30a961bc114286332f6a588de098e413a288ec398931de6409ea74900c3cf1437c9ec74af647720ff0e5977b24a83b5de16dd15d1f03f162fe08aefe4",
                             "046727173c506d3fd2bdbeebb83a83c562a18f02f3dcc0c00691a0e65ce090d7b3b4c0b858ad1b03462e766025deb21473b9e7fe6dd0e6673c044eae117603b994",
                             "04e4e9bc1737ef6670204a8bc977520c9f71b6fad1e4394c354156919619d903a2fc9c09a1c476a8112f01dad98c8f48a71410114623c1b24dcdc37f33c0ce8d0e",
                             "049364c5a01973070da0b0015c62b14edc76441cf1c6dafc5704bdf819023dfb83dd210a1d51f2dfab2da78fae68edf406cd27b1e40c317c21496eb19d23acaaec",
                             "04ecd8953555d3a1a41886402307c02da833281cf839b984d315486537f3ba2f8b51331f5aefd80a2421802bc8a2af92f9b4663f6dc4b05d4de1e958600f5378fe",
                             "04d496602cd63bfc06b1285a4675ecffffba4e7a104e0f90c3abe200889576c000bb08b445652c982af43d3cfdb3fa2dead5c6f073f911b8d885d9abe4cca2ca17",
                             "0478b665e21f1070146dca8bea51c8928c4b1b3fdc3c7c15812d937c0651473f1ea51a0214437972931a2427d46c944c45a6e0341e1ee6bbad832be873a6f48216",
                             "044ca7e1a651d6bbad2cd47ab9c0e33aad7bea4163943598bb5cbafd9170e8f35050f589ac8193f990f19b73130b7704fc33d34c8e2f9ef7e14f03557a7a8ba5af"
                            };

static CBlock CreateGenesisBlock(const CScript& genesisInputScript, const std::vector<CScript>& genesisOutputScript,
                                 uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion,
                                 const std::vector<CAmount>& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(genesisOutputScript.size());

    txNew.vin[0].scriptSig = genesisInputScript;

    assert(genesisOutputScript.size() == genesisReward.size());
    for(uint i = 0; i < genesisOutputScript.size(); i++)
    {
        txNew.vout[i].nValue = genesisReward[i];
        txNew.vout[i].scriptPubKey = genesisOutputScript[i];
    }

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();

    //add 4 points about pos to genesis block
    genesis.generationSignature = std::string("442c29a4d18f192164006030640fb54c8b9ffd4f5750d2f6dca192dc653c52ad");
    genesis.baseTarget = 153722867;
    genesis.pubKeyOfpackager = std::string("Scientific distribution of wealth to each one");
    genesis.cumulativeDifficulty = uint256();

    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion,
                                 const std::vector<CAmount>& genesisReward)
{
    const char* pszTimestamp = "... choose what comes next.  Lives of your own, or a return to chains. -- V";
    const CScript genesisInputScript = CScript() << 0x1f00ffff << CScriptNum(522) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));

    std::vector<CScript> genesisOutputScripts;
    for(uint i = 0; i < GENESISCOIN_CNT; i++)
    {
        const CScript genesisOutputScript = CScript() << ParseHex(baseAddr[i]) << OP_CHECKSIG;
        genesisOutputScripts.push_back(genesisOutputScript);

    }

    return CreateGenesisBlock(genesisInputScript, genesisOutputScripts, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Build genesis block for testnet.  In Imcoin, it has a changed timestamp
 * and output script (it uses Bitcoin's).
 */
static CBlock CreateTestnetGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion,
                                        const std::vector<CAmount>& genesisReward)
{
    const char* pszTimestamp = "The Times 14/June/2009 2018 FIFA World Cup Russia Starts.";
    const CScript genesisInputScript = CScript() << 0x1d00ffff << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));

    const CScript genesisOutputScript0 = CScript() << ParseHex("04568a0f5331da25101fe0fa27903d53d007ce88957754af97e680624d98a16b47b681b012d37f126db95e9e34984e18d455141572cf64a2b7ab078000e7f4f6da") << OP_CHECKSIG;
    const CScript genesisOutputScript1 = CScript() << ParseHex("04633a310e85a506f2beecfdbdaaaee389969c7a61e9b4a4ec643d1a0eb689f7baa5fe46dca2b008f54b33531340032881753c44867e4bd8787dcc3c7adde7e52c") << OP_CHECKSIG;
    const CScript genesisOutputScript2 = CScript() << ParseHex("04504d1eba1d002949543a461904701f37a20f81bb14ca6fb1c3be29af320f6f4e5d1dbb73ed31c55719507af59b012b2860452eb951c27f4dec58ab4ac0f0e5ef") << OP_CHECKSIG;

    std::vector<CScript> genesisOutputScript;
    genesisOutputScript.push_back(genesisOutputScript0);
    genesisOutputScript.push_back(genesisOutputScript1);
    genesisOutputScript.push_back(genesisOutputScript2);

    return CreateGenesisBlock(genesisInputScript, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 227931;
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPodsTargetSpacing = 60;
        consensus.nPodsAheadTargetDistance = 1;
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141 and BIP143)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 0; // Never / undefined

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x69;
        pchMessageStart[1] = 0x6d;
        pchMessageStart[2] = 0x63;
        pchMessageStart[3] = 0x6d;
        nDefaultPort = 8668;
        nPruneAfterHeight = 100000;

        std::vector<CAmount> genesisReward;
        for(uint i = 0; i < GENESISCOIN_CNT; i++)
        {
            genesisReward.push_back(MAX_MONEY / 100);
            if (i > GENESISCOIN_CNT - GENESISLOCK_ADDRCNT - 1)
                consensus.genesislockCoinAddr[i + GENESISLOCK_ADDRCNT - GENESISCOIN_CNT] = baseAddr[i];
        }
        consensus.genesislockCoinHeight = GENESISLOCK_MATURITY;

        genesis = CreateGenesisBlock(1529042124, 95609, 0x1f00ffff, 1, genesisReward);
        consensus.hashGenesisBlock = genesis.GetHash();
        consensus.hashGenesisTx = genesis.hashMerkleRoot;
        consensus.genesisCumulativeDifficulty = genesis.cumulativeDifficulty;
        consensus.genesisBaseTarget = genesis.baseTarget;
        assert(consensus.hashGenesisBlock == uint256S("000005068d80c1bdaa176f2d0d7fc2bbef4a45917e7d1924e7329d69e0d21f01"));
        assert(genesis.hashMerkleRoot == uint256S("772c6eb3628ab6e036763de6236c72f22e5863c6ec2a28c1556a6c4895e3d4b8"));

        vSeeds.push_back(CDNSSeedData("imorpheus.io", "dnsseed.imorpheus.io", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,63);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
                boost::assign::map_list_of
                ( 0, uint256S("000056da68a4ea9dc688ab0a3a9ee2c2432931f65c801a3a5c6a200ba204edb3")),
                1529042124, // * UNIX timestamp of last checkpoint block
                1441814,   // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
                60000.0     // * estimated number of transactions per day after checkpoint
    };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        consensus.BIP34Height = 21111;
        consensus.BIP34Hash = uint256S("0x0000000023b3a96d3484e5abb3755c413e7d41500f8e2a5c3f0dd01299cd8ef8");
        consensus.nPodsTargetSpacing = 60;
        consensus.nPodsAheadTargetDistance = 1;
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141 and BIP143)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800; // May 1st 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800; // May 1st 2017

        pchMessageStart[0] = 0x69;
        pchMessageStart[1] = 0x6d;
        pchMessageStart[2] = 0x63;
        pchMessageStart[3] = 0x74;
        nDefaultPort = 18668;

        nPruneAfterHeight = 1000;

        std::vector<CAmount> genesisReward;
        genesisReward.push_back(MAX_MONEY / 10 * 2);
        genesisReward.push_back(MAX_MONEY / 10 * 2);
        genesisReward.push_back(MAX_MONEY / 10 * 6);

        genesis = CreateTestnetGenesisBlock(1529042124, 70242, 0x1f00ffff, 1, genesisReward);
        consensus.hashGenesisBlock = genesis.GetHash();
        consensus.hashGenesisTx = genesis.hashMerkleRoot;
        consensus.genesisCumulativeDifficulty = genesis.cumulativeDifficulty;
        consensus.genesisBaseTarget = genesis.baseTarget;
        //assert(consensus.hashGenesisBlock == uint256S("00007ac2cf86fb52f10a3ac6c9c1586b0dfbf38e710476fcf23b506e6319f831"));
        //assert(genesis.hashMerkleRoot == uint256S("82b10867bff7b43ed801c987f783af7e57885a485c2b6e243cd187796c7e9dfa"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        // vSeeds.push_back(CDNSSeedData("testnetbitcoin.jonasschnelli.ch", "testnet-seed.bitcoin.jonasschnelli.ch", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (CCheckpointData) {
                boost::assign::map_list_of
                ( 546, uint256S("000000002a936ca763904c3c35fce2f3556c559c0214345d31b1bcebf76acb70")),
                1337966069,
                1488,
                300
    };

    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.nPodsTargetSpacing = 60;
        consensus.nPodsAheadTargetDistance = 1;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        pchMessageStart[0] = 0x69;
        pchMessageStart[1] = 0x6d;
        pchMessageStart[2] = 0x63;
        pchMessageStart[3] = 0x72;
        nDefaultPort = 18669;

        nPruneAfterHeight = 1000;

        std::vector<CAmount> genesisReward;
        genesisReward.push_back(MAX_MONEY / 10 * 2);
        genesisReward.push_back(MAX_MONEY / 10 * 2);
        genesisReward.push_back(MAX_MONEY / 10 * 6);

        genesis = CreateTestnetGenesisBlock(1296688602, 2, 0x207fffff, 1, genesisReward);
        consensus.hashGenesisBlock = genesis.GetHash();
        consensus.genesisCumulativeDifficulty = genesis.cumulativeDifficulty;
        consensus.genesisBaseTarget = genesis.baseTarget;
        //assert(consensus.hashGenesisBlock == uint256S("0x0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"));
        //assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData){
                boost::assign::map_list_of
                ( 0, uint256S("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")),
                0,
                0,
                0
    };
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
        return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
        return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

