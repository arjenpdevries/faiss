load json;
load vss;
CREATE TABLE input AS SELECT docid, vector FROM 'conformanceTests/msmarco-passage-openai-ada2/?.jsonl.gz';
ALTER TABLE input ALTER vector TYPE float[1536];
CREATE INDEX cos_idx ON input USING HNSW (vector) WITH (metric = 'cosine');
SELECT * FROM input ORDER BY array_cosine_similarity(vector, [-0.0017241806, -0.005905385, -0.008286016, -0.022072354, -0.037009925, 0.025582897, -0.0033311064, -0.015378163, -0.003627797, -0.005145005, 0.019457214, 0.012691959, 0.0177659, -0.0043490925, -0.0038445408, 0.006964233, 0.021276442, -0.0011849855, 0.009494097, 0.010865625, -0.009145886, -0.0028052356, 0.0080017615, -0.013892934, 0.044684794, -0.01089405, 0.025469195, -0.040676806, 0.015761906, -0.003535414, 0.012016855, -0.0057952367, -0.0075682737, -0.02930663, -0.037322603, -0.01807858, -0.024346389, -0.0089895455, 0.0020199828, -0.015022845, -0.0069819987, 0.018433899, -0.011405708, 0.012805661, -0.022768779, -0.01191026, -0.0019560256, -0.011839196, -0.008349973, 0.025327068, 0.020196276, -0.00033777417, -0.019087683, 0.011689963, 0.011960004, -0.0022740352, -0.019542491, -0.0002926932, -0.008136783, -0.018504962, 0.02220027, 0.015477653, 0.014795442, 0.015335525, -0.015790332, -0.018249134, -0.011931579, -0.0010863848, 0.017964879, 0.0012684853, 0.019997297, 0.04570811, 0.0039760084, -0.02350784, 0.0030148732, -0.015548716, -0.026719915, -0.028667057, -0.048493803, -0.0002005326, -0.017183179, -0.0050277496, -0.028567567, -0.011064603, -0.0014976654, -0.009337757, -0.002902948, 0.022086566, -0.01739637, -0.009501204, 0.010481881, 0.016543606, 0.0076251244, 0.031694368, -0.0225698, 0.0036846476, -0.01985517, 0.014624889, 0.006431256, -0.032774534, 0.008278909, 0.029903563, -0.019258237, -0.031523813, -0.01473859, -0.011768132, -0.0070495093, -0.008641334, 0.009934692, -0.006562724, 0.007237828, -0.0047292826, -0.0014008413, -0.028397014, -0.015861396, -0.039312385, -0.056907732, -0.02947718, 0.01624514, -0.04937499, 0.027800081, -0.025611322, 0.010702178, -0.00548611, 0.014347741, -0.0015793885, -0.03689622, 0.005589152, -0.010211839, -0.021560697, 0.018206494, -0.0037770304, 0.015065484, -0.014795442, 0.0014434794, 0.0077104005, -0.03939766, -2.2637643e-05, -0.014980207, -0.017126327, 0.004736389, 0.017964879, 0.0069677862, 0.02104904, 0.00086964085, 0.011412814, 0.021972865, 0.024531156, 0.002131908, -0.026080342, 0.024545368, -0.0074901036, 0.01833441, -0.011846302, 0.029960414, 0.033456743, -0.00254763, 0.01928666, 0.01901662, 0.003345319, -0.01196711, -0.007170317, -0.0025902682, -0.009614905, 0.010645327, 0.042297054, -0.0006915377, -0.01860445, 0.013836083, 0.024076348, 0.0058947257, 0.02706102, -0.035588652, -0.005510982, 0.018050155, 0.024929112, -0.007355083, 0.04388888, -0.041103188, -0.01118541, -0.008271803, 0.033001937, 0.020565806, 0.046958826, 0.0038196687, -0.010702178, 0.035588652, -0.00602264, 0.012819874, -0.028638631, 0.0034732334, -0.00494958, 0.0024392582, 0.0036633287, -0.57169247, -0.024516942, 0.0049886648, -0.035076994, -0.0066195745, 0.04539543, -0.008520526, -0.0034412548, -0.025739236, -0.0073906146, -0.021461207, 0.02821225, 0.0007750374, -0.022328183, -0.018732365, -0.018675514, -0.010353967, -0.028809184, 0.035503376, 0.009366183, 0.0076677627, 0.00169842, -0.00082789094, 0.005997768, 0.004572943, 0.0014870059, 0.02110589, -0.019656193, -0.0137437, -0.00041216888, -0.031097433, 0.021603335, -0.013317319, 0.011412814, 0.035560224, 0.027714804, -0.017623773, 0.020096786, 0.02507124, 0.027984846, -0.040477827, 0.0024428114, 0.032632407, 0.021702824, -0.005148558, 0.017680624, 0.019755682, -0.0061256825, 0.026819402, -0.033343043, 0.0076890816, -0.013246255, -0.009963117, -0.01604616, 0.003961796, -0.011867621, -0.0044912198, -0.004445028, 0.0063282135, 0.038317494, -0.008435249, 0.010304222, -0.027473189, 0.00021863161, -0.025852937, -0.014091912, -0.010027074, 0.006083044, 0.024076348, -0.0075753797, 0.017950665, 0.016273566, -0.0027537146, -0.02690468, 0.0072911256, -0.0034217124, -0.005308451, 0.0064881067, 0.019684618, -0.020793209, -0.015904034, -0.011476772, -0.004974452, 0.00096824154, 0.04064838, -0.006282022, -0.020622658, -0.007138339, -0.0066018086, 0.0052551534, 0.004459241, 0.018789217, 0.014091912, -0.012990426, -0.0007816996, 0.022058142, 0.001845877, 0.011604686, 0.0044699004, -0.0151223345, -0.0036846476, -0.013594467, 0.017140541, -0.009515416, 0.026066128, 0.012954894, -0.0022331737, -0.024261113, 0.030216243, -0.045651257, 0.0027181827, -0.0046617724, -0.015918247, 0.010851412, 0.0041394546, -0.038914427, 0.018760791, 0.012649321, -0.006928701, -0.021276442, 0.01081588, 0.012208726, 0.01749586, -0.020693721, -0.0029544693, 0.009820989, 0.020054149, -0.024360603, -0.02032419, 0.016785223, 0.005763258, 0.020565806, 0.049147587, -0.011860515, 0.0071667642, 0.01891713, -0.0062251715, 0.00095758203, -0.014596464, -0.011171198, -0.041586418, 0.013381276, -0.0014203837, -0.021702824, -0.015449227, -0.026677275, 0.0026258, 0.00038907322, -0.017893815, 0.019840958, 0.03135326, -0.007159658, -0.024389029, -0.01604616, -0.02016785, -0.024943324, 0.0075043165, -0.0107732415, 0.029335054, -0.0064596813, 0.017339518, 0.018149644, -0.027231572, -0.0073621892, -0.00034310395, -0.025312854, 0.006608915, 0.02571081, 0.045082748, -0.010595583, 0.007980443, -0.0045800493, -0.002872746, 0.007284019, 0.007120573, 0.01536395, 0.0024694602, -0.015818758, 0.00950831, 0.0009327098, -0.0002900283, -0.016586244, -0.0061931927, -0.023323074, 0.012272684, -0.012009748, 0.015818758, -0.018576026, -0.008037293, -0.00880478, 0.01839126, 0.0047932398, 0.016586244, 0.014198507, 0.036384564, 0.01227979, 0.021916015, 0.022498736, 0.016131438, 0.004931814, 0.029875139, -0.0027999058, 0.045537557, -0.01444723, 0.016742585, -0.026108768, 0.010652434, -0.018305983, 0.014710166, -0.0018778556, -0.0025458536, -0.004576496, 0.010993539, -0.021816526, 0.015761906, 0.018803429, -0.01368685, 0.014305103, -0.006200299, -0.0010002202, -0.029647734, -0.0023042373, -0.007024637, -0.010624008, -0.028411228, -0.0059657893, 0.0070495093, -0.012294003, 0.019869383, 0.0039404766, 0.0070992536, 0.017808538, 0.0137437, -0.0035727224, 0.011533623, -0.006964233, -0.0024854494, -0.0044663474, 0.029931989, 0.002746608, 0.0046688784, 0.0005805008, 0.013800551, -0.0056531094, 0.036384564, -0.027075233, 0.02283984, -0.002014653, -0.015577141, 0.0499435, -0.0068149995, 0.013558935, -0.018320197, 0.015548716, 0.026677275, -0.01058137, 0.026620425, 0.023010394, 0.018050155, 0.021603335, -0.0033737444, -0.013160979, -0.00570996, -0.01881764, -0.006083044, 0.003276032, 0.0053759613, 0.02497175, -0.02362154, -0.016998414, -0.0027927996, -0.0061256825, 0.004974452, -0.005823662, 0.0015545163, -0.014866506, 0.004775474, 0.0021994184, 0.004310007, -0.02131908, -0.02821225, -0.0012818098, 0.0038338813, 0.033740997, -0.007831208, -0.013509191, -0.015804544, 0.002499662, -0.011277794, 0.029704586, -0.0054718973, 0.0039369236, 0.009401714, -0.008172314, -0.015591354, 0.004739942, 0.022910904, -0.001189427, -0.0151223345, -0.030102542, -0.006001321, 0.00075638323, 0.00880478, -0.0071312324, 0.04963082, -0.005777471, -0.004203412, -0.016842073, -0.007916485, -0.01860445, -0.017879602, -0.010666647, -0.01105039, -0.011355964, 0.0036455628, -0.02717472, -0.016131438, -0.004064838, 0.06009138, -0.0010393051, -0.012635108, -0.060944144, 0.007539848, -0.02549762, 0.15133704, 0.0057170666, -0.024303751, 0.018746577, -0.014624889, -0.004850091, -0.02215763, -0.03840277, 0.031893343, -0.0011379059, 0.011718388, 0.0046830913, 0.005638897, 0.0075611672, -0.0031623302, 0.011839196, -0.0049211546, -0.037550006, -0.020110998, -0.017780112, -0.012485875, -0.022967756, 0.02524179, 0.049403414, -0.02058002, -0.0043171137, 0.0040008808, 0.038630173, 0.009934692, -0.0087905675, 0.020906912, 0.01604616, 0.0062749158, 0.0047115167, 0.007305338, 0.01839126, 0.00602264, 0.027970633, 0.021489633, 0.010083925, 0.026378809, -0.015278675, -0.009671756, -0.028297527, 0.01817807, -0.01429089, 0.016202502, -0.006001321, 0.008349973, -0.021390144, 0.0307279, -0.0022598226, -0.0010393051, 0.010872731, 0.0064881067, 0.026790978, 0.004832325, 0.01353051, -0.017168965, -0.0059515764, -0.027132083, -0.048607502, 0.02278299, -0.008840312, 0.0014710166, -0.054207314, -0.009884947, 0.0010517413, 0.0023148968, -0.015705056, 0.03035837, -0.027515827, -0.026876254, 0.0012755917, 0.012265578, 0.008158102, 0.015747694, -0.002863863, 0.013928466, -0.0039546895, -0.029221352, -0.049602393, 0.005887619, -0.025739236, 0.0055358545, 0.0034750102, -0.0011316879, -0.012052387, -0.0068540843, 0.01745322, -0.02220027, 0.024246901, 0.009209842, 0.0018920684, 0.001189427, 0.009550948, 0.0007315109, 0.00950831, -0.000100377336, -0.0041856463, 0.011867621, -0.010751923, 0.004619134, -0.005361749, 0.0062393844, 0.015591354, 0.0015873832, 0.014070593, -0.022171844, -0.024346389, 0.0091885235, -0.020224702, -0.003848094, -0.022726139, -0.0053866208, 0.007781464, 0.005443472, -0.023664178, -0.04394573, -0.014106125, -0.0038729662, -0.0082078455, 0.0028265547, 0.030386796, -0.038146943, 0.03254713, -0.00940882, 0.00247479, -0.025625534, -0.0155202905, -0.015164972, 0.02016785, 0.003610031, -0.0019613553, -0.008918482, 0.007880953, 0.0047825803, 0.019542491, -0.00185476, 0.0063637453, -0.0024410347, -0.015236036, -0.011434133, -0.022484524, -0.031580664, -0.017723262, 0.008556058, 0.0017534944, -0.0075682737, 0.023138309, -0.022356609, 0.00028403234, -0.010801667, 0.008854525, -0.005624684, -0.06054619, -0.0267057, -0.016529394, 0.03135326, 0.013914254, 0.031637516, 0.022242907, 0.04326352, 0.033684146, 0.023365712, -0.014866506, 0.012457449, 0.003904945, -0.012755916, -0.013580254, 0.017822752, 0.015349738, -0.0008985104, -0.029789861, 0.0025494066, 0.006729723, 0.0145254005, -0.021546483, 0.0005218733, -0.04053468, 0.011874728, -0.014809654, 0.007383508, 0.0026204702, -0.020764785, -0.002986448, 0.060944144, 0.031978622, 0.00564245, 0.002483673, 0.03459376, -0.029050801, 0.048237972, 0.00024161625, -0.0047612614, -0.034480058, -0.002634683, 0.004622687, -0.002053738, 0.013381276, -0.0027892464, 0.006910935, 0.011796557, 0.010282903, 0.00020675066, 0.012564044, -0.027032593, 0.0036739882, -0.0025032153, -0.027629528, 0.01392136, -0.019926233, -0.019940447, -0.030301519, 0.015875608, -0.030130967, -0.0055571734, 0.022811417, -0.021873377, -0.04377518, 0.0046369, -0.035275973, 0.0097854575, 0.007518529, 0.029036587, -0.0014141657, -0.024332177, -0.018533386, 0.0040186467, -0.025483407, 0.0074901036, 0.0059657893, 0.005390174, -0.016898924, -0.019940447, -0.025454981, -0.004572943, -0.017680624, -0.0004583602, 0.019172959, 0.022015505, 0.0032422768, 0.019059258, -0.003085937, 0.010993539, -0.00059782254, -0.011377282, -0.04027885, -0.020366829, -0.011974216, -0.03067105, -0.002105259, -0.015292887, 0.019670404, -0.0040932633, -0.03573078, -0.0037628177, -0.015790332, 0.0071774237, 0.03459376, -0.02141857, -0.0010277573, -0.017950665, 0.0032120747, -0.026847828, 0.020224702, -0.04027885, 0.00841393, -0.024630643, 0.031779643, -0.024062136, 0.0031960856, -0.0036775414, 0.0055998117, 0.011732601, 0.0015847184, -0.0052196216, 0.009792564, -0.013168085, 0.018462323, 0.003965349, -0.014951781, -0.0083926115, -0.009216949, 0.0067155105, -0.01651518, -0.006392171, -0.008087038, 0.00494958, -0.016159862, -0.0041536675, -0.016742585, -0.013850296, 0.006381511, 0.0058663003, 0.00091494387, -0.0281554, 0.021034826, 0.0066373404, -0.010041287, -0.0034128295, 0.017694836, -0.038061664, 0.011526516, 0.01630199, -0.016756797, 0.04630504, -0.007816996, -0.0039404766, -0.0074901036, 0.027075233, -0.0013164532, 0.014020848, -0.00016600013, 0.0060084276, 0.007454572, 0.011192517, 0.006409937, -0.0151223345, 0.013658424, 0.008826099, 0.019414576, 0.0040008808, 0.004111029, -0.0036882008, 0.0022935777, 0.029562458, 0.00802308, -0.021461207, 0.01708369, -0.009529629, 0.02053738, -0.016941562, 0.011881834, -0.0077459323, -0.034934867, 0.012023961, 0.0032564895, -0.017510071, -0.009415927, 0.002881629, 0.027515827, 0.017680624, 0.02768638, -0.009302225, -0.0036420096, -0.0034767867, 0.021191167, -0.03919868, -0.0008554281, -0.008143889, 0.04076208, 0.01052452, -0.013878722, -0.029590884, -0.025142303, -0.028084336, -0.020338403, -0.0046120277, 0.002131908, 0.028340165, 0.023763668, -0.018803429, 0.01749586, -0.010922476, -0.0074119335, 0.02549762, 0.005756152, 0.03354202, -0.021873377, -0.013082809, 0.0014861176, -0.001845877, 0.024644857, 0.02188759, -0.019343512, 0.0241332, 0.0023806307, 0.024985962, -0.006921595, 0.0030930433, 0.0027341722, -0.013509191, 0.005191196, 0.02058002, -0.015065484, 0.012400598, 0.012173194, -0.010019967, 0.0016788776, -0.0017321752, 0.0006688861, 0.009415927, -0.008620014, 4.4942175e-05, -0.012962, 0.01003418, -0.022768779, 0.022541374, 0.0024214922, 0.005447025, 0.042723436, -0.013367063, -0.012855405, -0.011988429, -0.006409937, -0.022910904, 0.0022438332, -0.003435925, -0.0026240235, 0.012492981, 0.023024607, -0.0005218733, -0.036498267, -0.014219827, -0.006893169, 0.016998414, -0.010041287, 0.016429905, 0.02680519, -0.021574909, 0.010346861, -0.02455958, -0.0051201326, -0.023394138, -0.031267986, -0.012031068, -0.0076038055, 0.015420801, 0.019840958, -0.021773888, -0.014489869, -0.004967346, 0.018902918, -0.007404827, 0.010943795, 0.19659035, -0.013374169, 0.021560697, 0.04775474, -0.02879497, 0.0026968636, 0.010112351, 0.014141656, -0.0047115167, 0.018419685, -0.023649966, 0.012492981, -0.02131908, 0.011427027, 0.0027590443, -0.029278204, -0.007831208, -0.030216243, -0.016699946, -0.02507124, 0.009494097, 0.010083925, 0.007674869, -0.002105259, 0.035275973, 8.233384e-05, -0.016557818, 0.013956891, -0.01561978, 0.025156515, -0.03547495, 0.022811417, 0.0145254005, -0.012450343, -0.016472543, 0.0074261464, 0.011789451, 0.0025849384, -0.006552064, 0.012919363, -0.018590238, -0.0057348325, 0.0053724083, -0.020707933, -0.017026838, 0.0044947728, -0.00055873755, -0.022541374, -0.025227578, 0.0023948434, -0.031211134, -0.0011023742, 0.01938615, 0.029931989, 0.0019187173, 0.013679743, 0.0074901036, 0.020594232, 0.0025689492, -0.018760791, -0.0067332764, 0.035588652, -0.022285545, 0.00788806, 3.2450527e-05, 0.046106063, 0.003610031, 0.03297351, 0.007532742, -0.020253126, 0.013146766, -0.014354847, -0.029278204, 0.003306234, -0.019087683, -0.054264165, 0.0355318, 0.04133059, -0.014980207, 0.016969988, -0.025156515, 0.00077947887, -0.0011450123, 0.006374405, -0.0111640915, -0.007188083, 0.022598226, -0.004025753, -0.02862442, 0.013303106, -0.008506313, -0.025511833, -0.009423033, 0.0023717477, 0.022285545, -0.011270687, -0.009323545, 0.008712398, -0.0025618428, 0.037976388, -0.0072662532, 0.04533858, 0.013445233, 0.005479004, -0.0022260672, 0.0034945526, -0.0021656633, -0.016813649, -0.0061896397, -0.01817807, -0.030841602, 0.010744817, -0.00084610103, -0.0060937037, 0.01097222, 0.0076038055, 0.0055536204, -0.018874492, -0.00089673384, -0.001150342, -0.009714394, -0.042410757, 0.0075682737, 0.009465672, -0.016756797, 0.019542491, -0.028382802, 0.012222939, -0.00786674, -0.030102542, 0.00019164965, -0.00048722982, 0.045025896, -0.00046369, -0.0019595788, 0.027714804, -0.010332648, 0.0011538952, -0.021461207, -0.006083044, -0.031211134, -0.008342867, -0.00072973437, 0.009238268, 0.028254887, -0.024516942, 0.033314615, 0.0009913372, -0.02998884, -0.008691078, -0.00786674, -2.4136642e-05, -0.0065378514, 0.01833441, 0.013864509, 0.003570946, -0.0035549568, -0.030955305, 0.0004450358, 0.000111147914, -0.033939976, 0.0019098343, 0.02481541, -0.021546483, -0.004263816, -0.005379515, -0.18112691, 0.00734087, 0.027885357, -0.012187407, 0.014951781, -0.01429089, 0.017836964, 0.0105671575, -0.01306149, -0.02889446, 0.028283313, 0.023422563, -0.038630173, 0.012073706, -0.0072094025, 0.01167575, -0.022740353, 0.031637516, -7.894722e-05, 0.011704175, 0.012564044, -0.026236681, -0.022072354, -0.020139424, 0.02998884, 0.020452105, -0.00090161944, 0.013160979, 0.0022722587, 0.0019098343, 0.0029580223, 0.0022491629, 0.004818112, -0.008158102, 0.0013626446, -0.000301132, -0.006036853, 0.006829212, 0.015818758, 0.013473659, 0.029818287, 0.003979562, 0.04053468, -0.013381276, -0.010986432, 0.015079696, 0.0052764723, 0.021276442, 0.043434072, 0.022541374, -0.039511364, -0.031637516, -0.0036988605, 0.014795442, 0.01792224, 0.022811417, -0.0002182985, 0.009060609, -0.011654431, 0.0087834615, 0.008030187, -0.016230926, 0.011704175, -0.010027074, 0.007003318, -0.00979967, -0.02131908, 0.011121454, -0.018490748, 0.010055499, -0.0017721485, -0.03678252, 0.012819874, -0.020679507, -0.0047861338, 0.005425706, -0.02497175, 0.017808538, -0.001598931, 0.0075114225, -0.046930403, 0.037493154, -0.0056317905, 0.0014079476, -0.0024907792, 0.012947788, -0.013125448, -0.0014070593, -0.03246185, -0.0072094025, -0.0022456099, -0.04474164, -0.02001151, -0.011533623, 0.027103657, 0.042865563, -0.026648851, 0.010197626, 0.013089916, -0.0010499647, -0.016500968, -0.004242497, -0.01959934, 0.022697715, 0.011498091, -0.0035869353, 0.0038019028, 0.012372173, 0.03783426, -0.0023717477, -0.015605567, 0.013459446, 0.011135667, 0.03035837, 0.0083926115, 0.029391905, -0.0073266574, -0.015690843, 0.002062621, 0.0026862042, 0.05861326, 0.0037379453, 0.009565161, -0.011533623, 0.014347741, -0.004850091, -0.09573688, -0.039909318, 0.018405473, 0.007045956, 0.0067332764, 0.03689622, -0.030187817, 0.014269571, -0.023848945, 0.029249778, 0.0032351704, -0.033911552, 0.0042318376, -0.01151941, 0.028937098, -0.008229165, -0.024332177, -0.012798554, -0.015236036, 0.013786338, -0.011249368, -0.029050801, 0.004953133, -0.021759674, 0.0005951577, -0.011995536, -0.035076994, 0.032348152, 0.020722145, 0.017339518, -0.013281787, 4.058398e-05, -0.0011494538, 0.0012027514, -0.029619308, -0.007980443, -0.025454981, -0.0015065484, 0.046049215, -0.01928666, 0.00796623, -0.02419005, 0.0073337634, -0.022697715, -0.020238914, -0.03303036, -0.0011130336, 0.009870734, 0.02706102, -0.0076677627, -0.025696598, -0.009216949, -0.02168861, -0.026165618, -0.00903929, 0.011341751, 0.0067510423, 0.043490924, -0.011263581, 0.029590884, -0.0016673297, -0.005972896, -0.017794326, 0.0037983495, 0.0307279, 0.010027074, -0.004221178, -0.005248047, -0.004182093, -0.008150995, 0.009096141, 0.012663534, -0.032632407, 9.681028e-06, -0.03004569, -0.02743055, -0.051848, -0.019130321, 0.0306142, -0.055628587, -0.020309977, -0.004228284, -0.008769249, 0.016259352, 0.014049274, 0.0068007866, -0.0036846476, -0.008691078, 0.0042353906, -0.012940682, 0.006651553, 0.019556703, 0.027259998, -0.00076260127, -0.0024641303, 0.0010801668, 0.023834731, 0.020864272, 0.004771921, 0.014909144, -0.02440324, -0.005546514, -0.07663499, 0.049517117, -0.0017099679, -0.010552945, 0.005560727, 0.009558055, -0.047100954, -0.046703, 0.0013741924, 0.012528513, -0.029619308, 0.03820379, 0.0051876428, -0.02278299, -0.012137663, -0.040932633, 0.014553825, 0.009870734, 0.022754565, 0.026293533, 0.002417939, 0.000987784, 0.032802958, 0.006434809, -7.1230155e-05, 0.005443472, -0.00041683242, 0.031381685, -0.011526516, 0.0054576844, 0.0003597595, -0.024204263, -0.005166324, -0.0089895455, -0.011931579, -0.005621131, -0.01645833, 0.019983085, 0.03479274, 0.03516227, -0.0071774237, -0.03149539, -0.017623773, -0.0016966434, -0.01298332, 0.014894931, -0.004995771, 0.020892698, 0.0025387472, 0.017652199, -0.0068221055, 0.006132789, -0.01886028, -0.014781229, -0.013651318, -0.024644857, 0.02534128, 0.017467434, -0.0153213125, -0.030415222, 0.030329945, 0.00054896635, 0.038061664, -0.0013075703, 0.02063687, -0.010304222, 0.015250249, 0.030187817, -3.9334813e-05, -0.027700592, -0.012471662, -0.008015974, 0.002236727, 0.015136547, 0.015534503, -0.01405638, 0.0020466316, 0.0274874, -0.0306142, 0.024516942, 0.0042531565, -0.030329945, -0.032149173, 0.033115637, 0.041103188, 0.012606683, -0.014667527, 0.024758559, -0.031324834, 0.032888234, 0.001640681, 0.014781229, -0.010439243, -0.029562458, -0.0193293, 0.0099702235, -0.008826099, 0.012421917, 0.011121454, 2.5427446e-05, 0.027942209, -0.01157626, -0.0099844355, -0.020594232, -0.0058663003, -0.00016067036, -0.0071774237, -0.013224936, -0.034252655, -0.0036597755, 0.015761906, 0.032632407, -0.02565396, 0.015406589, -0.024019497, 0.027601102, 0.013708169, -0.020821635, -0.014667527, 0.017026838, 0.027515827, -0.0074687847, 0.014695953, 0.01105039, 0.014404592, 0.019258237, 0.00016999745, -0.016799435, 0.0010677306, 0.011306219, 0.00047390538, -0.0007643779, 0.014099019, 0.0063033416, 0.009906266, -0.008811886, 0.005646003, 0.021645973, -0.017595347, 0.06372984, 0.011604686, -0.0025227577, 0.03422423, 0.011618899, 0.01429089, 0.005109473, -0.0027359487, -0.016003523, -0.015477653, 0.031523813, 0.00532977, 0.01536395, -0.03285981, 0.004658219, 0.01196711, 0.003007767, 0.018320197, -0.013516297, -0.013324426, 0.0037841368, 0.016472543, 0.020110998, -0.0037628177, -0.028041696, -0.02001151, 0.04243918, -0.031381685, -0.017097902, -0.03186492, 0.006409937, 0.0068398714, -0.027672166, -0.024445878, 0.0036331266, -0.023920009, 0.017439008, -0.006431256, 0.022967756, -0.0019204938, -0.029335054, 0.01969883, -0.03666882, -0.009366183, 0.046219766, 0.016444117, -0.011554942, 0.017311092, -0.011896047]::FLOAT[1536]) LIMIT 3;

